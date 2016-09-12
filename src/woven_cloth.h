#pragma once
/* A shader model for woven cloth
 * By Vidar Nelson and Peter McEvoy
 * Usage:
 *
 * Before rendering, fill out an instance of wcWeaveParameters
 * with the desired parameters to the model.
 * Then call wcWeavePatternFromFile to load a weaving pattern.
 *
 * During rendering, fill out an instance of wcIntersectionData
 * with the relevant information and call wcShade to recieve the
 * reflected color.
 *
 * After rendering, call wcFreeWeavePattern to free the memory
 * used by the weaving pattern.
 */

#include <stdint.h>
#include "wif/wif.h"

// This macro occurs in front of all functions, and can be used if
// the code is intended to be run as CUDA, for instance
// TODO(Vidar): Check if there's any way around this...
#ifndef WC_PREFIX
#define WC_PREFIX
#endif


// Used when the model has to return a color
typedef struct
{
    float r,g,b;
}wcColor;

typedef struct
{
// These are the parameters to the model
    float uscale;
    float vscale;
    float yarnvar_amplitude;
    float yarnvar_xscale;
    float yarnvar_yscale;
    float yarnvar_persistance;
    uint32_t yarnvar_octaves;
    uint8_t realworld_uv;

// These are set by calling one of the wcWeavePatternFrom* functions
// after all parameters above have been defined
    uint32_t pattern_height;
    uint32_t pattern_width;
    Pattern * pattern;
    float specular_normalization;
    float pattern_realheight;
    float pattern_realwidth;
} wcWeaveParameters;

// Intersection data to be set by the renderer
// The coordinate system for the directions is defined such that
// x points along dP/du and y points along dP/dv. z points along the normal
// dP/du and dP/dv are the derivatives of the current shading point with
// respect to the u and v texture coordinates, respectively. I.e. vectors
// which point in the direction of increasing u or v along the surface.
typedef struct
{
    float uv_x, uv_y;       // Texture coordinates
    float wi_x, wi_y, wi_z; // Incident direction 
    float wo_x, wo_y, wo_z; // Outgoing direction
	void *context;          // User data sent to callbacks
} wcIntersectionData;

// These are used to load a weaving pattern from a file
// Call _after_ all parameters have been set.
// Don't forget to free the weaving pattern when you're done rendering
WC_PREFIX
void wcWeavePatternFromFile(wcWeaveParameters *params, const char *filename);
WC_PREFIX
void wcWeavePatternFromFile_wchar(wcWeaveParameters *params,
    const wchar_t *filename);

WC_PREFIX
void wcFreeWeavePattern(wcWeaveParameters *params);

// This function will calculate the actual shading
WC_PREFIX
wcColor wcShade(wcIntersectionData intersection_data,
        const wcWeaveParameters *params);


// ========= Advanced usage =========

typedef struct
{
    uint32_t yarn_type;
    float normal_x, normal_y, normal_z; //(not yarn local coordinates)
    float u, v; //Segment uv coordinates (in angles)
    float length, width; //Segment length and width
    float x, y; //position within segment (in yarn local coordiantes). 
    uint32_t total_index_x, total_index_y; //index for elements (not yarn local coordinates). TODO(Peter): perhaps a better name would be good?
    uint8_t warp_above; 
    uint8_t yarn_hit; //True if we hit a yarn. False if we hit space inbetween. 
    uint8_t ext_between_parallel; //True if extensionis between to adjascent parallel yarns. -> bend should be zero
} wcPatternData;

WC_PREFIX
wcPatternData wcGetPatternData(wcIntersectionData intersection_data,
    const wcWeaveParameters *params);
WC_PREFIX
wcColor wcEvalDiffuse(wcIntersectionData intersection_data,
    wcPatternData data, const wcWeaveParameters *params);
WC_PREFIX
float wcEvalSpecular(wcIntersectionData intersection_data,
    wcPatternData data, const wcWeaveParameters *params);

WC_PREFIX
void wcWeavePatternFromData(wcWeaveParameters *params, uint8_t *warp_above,
	uint32_t *yarn_type, YarnType *yarn_types, uint32_t num_yarn_types,
	uint32_t pattern_width, uint32_t pattern_height);
/* wcWeavePatternFromFile calles one of the functions below depending on
 * file extension*/
WC_PREFIX
void wcWeavePatternFromWIF(wcWeaveParameters *params, const char *filename);
WC_PREFIX
void wcWeavePatternFromWIF_wchar(wcWeaveParameters *params,
    const wchar_t *filename);
WC_PREFIX
void wcWeavePatternFromWeaveFile(wcWeaveParameters *params, const char *filename);
WC_PREFIX
void wcWeavePatternFromWeaveFile_wchar(wcWeaveParameters *params,
    const wchar_t *filename);

/* This function needs to be called before rendering if any parameters have been
changed since reading the pattern*/
WC_PREFIX
void wcFinalizeWeaveParameters(wcWeaveParameters *params);


//for testing...
typedef struct
{
    float length, width; //Segment length and width
    float start_u, start_v; //Coordinates for top left corner of segment in total uv coordinates.
    uint8_t between_parallel;
    uint8_t warp_above;
    uint8_t yarn_hit;
} wcYarnSegment;

WC_PREFIX
wcYarnSegment wcCalculateSegmentDim(float total_u, float total_v,
        wcWeaveParameters *params);
