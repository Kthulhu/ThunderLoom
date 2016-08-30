#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/woven_cloth.h"

#define test(fn) \
        printf("\x1b[33m" # fn "\x1b[0m "); \
        test_##fn(); \
        puts("\x1b[1;32m ok \x1b[0m");

static wcWeaveParameters params_fullsize;
static wcWeaveParameters params_halfsize;
wcIntersectionData intersection_data;

static void test_sample_yarntype_with_uv() {
    wcWeaveParameters *params = &params_fullsize;

    //We can correctly use uv coords to sample yarn types.
    // Just a little sanity check.,.

    intersection_data.uv_y = 0.25;
    intersection_data.uv_x = 0.25;
    assert(wcGetPatternData(intersection_data, params).yarn_type == 2);
    intersection_data.uv_y = 0.25;
    intersection_data.uv_x = 0.75;
    assert(wcGetPatternData(intersection_data, params).yarn_type == 1);
    intersection_data.uv_y = 0.75;
    intersection_data.uv_x = 0.75;
    assert(wcGetPatternData(intersection_data, params).yarn_type == 2);
    intersection_data.uv_y = 0.75;
    intersection_data.uv_x = 0.25;
    assert(wcGetPatternData(intersection_data, params).yarn_type == 1);
}

static void test_sampling_with_thin_weft_and_warp() {
    wcWeaveParameters *params = &params_halfsize;

    //Can sample correct yarns, when both weft and warp are thin
    intersection_data.uv_y = 0.25;
    intersection_data.uv_x = 0.25;
    assert(wcGetPatternData(intersection_data, params).yarn_type == 2);
    assert(wcGetPatternData(intersection_data, params).yarn_hit == 1);
    assert(wcGetPatternData(intersection_data, params).warp_above == 0);
    intersection_data.uv_y = 0.25;
    intersection_data.uv_x = 0.75;
    assert(wcGetPatternData(intersection_data, params).yarn_type == 1);
    assert(wcGetPatternData(intersection_data, params).yarn_hit == 1);
    assert(wcGetPatternData(intersection_data, params).warp_above == 1);
    intersection_data.uv_y = 0.75;
    intersection_data.uv_x = 0.75;
    assert(wcGetPatternData(intersection_data, params).yarn_type == 2);
    assert(wcGetPatternData(intersection_data, params).yarn_hit == 1);
    assert(wcGetPatternData(intersection_data, params).warp_above == 0);
    intersection_data.uv_y = 0.75;
    intersection_data.uv_x = 0.25;
    assert(wcGetPatternData(intersection_data, params).yarn_type == 1);
    assert(wcGetPatternData(intersection_data, params).yarn_hit == 1);
    assert(wcGetPatternData(intersection_data, params).warp_above == 1);

    //Can sample between yarns. Returns no hit...
    intersection_data.uv_x = 0.1;
    intersection_data.uv_y = 0.1;
    assert(wcGetPatternData(intersection_data, params).yarn_hit == 0);
    intersection_data.uv_x = 0.5 - 0.1;
    intersection_data.uv_y = 0.5 - 0.1;
    assert(wcGetPatternData(intersection_data, params).yarn_hit == 0);
    
    //Adjascent warp is extended when weft is missed. 
    intersection_data.uv_x = 0.25; //(middle of weft segment, outside bottom of weft segment)
    intersection_data.uv_y = 0.25 + 0.2;
    assert(wcGetPatternData(intersection_data, params).yarn_hit == 1);
    assert(wcGetPatternData(intersection_data, params).yarn_type == 1);
    assert(wcGetPatternData(intersection_data, params).warp_above == 1);
    assert(wcGetPatternData(intersection_data, params).width == 1.f); //Half sized warp, remember width is defined to be across.
    assert(wcGetPatternData(intersection_data, params).length == 3.f); //1.5*2 sized warp because of extension...
    
    //Adjascent weft is extended when warp is missed. 
    intersection_data.uv_x = 0.75 - 0.2; //(middle of warp segment, outside left of warp segment)
    intersection_data.uv_y = 0.25;
    assert(wcGetPatternData(intersection_data, params).yarn_hit == 1);
    assert(wcGetPatternData(intersection_data, params).yarn_type == 2);
    assert(wcGetPatternData(intersection_data, params).warp_above == 0);
    assert(wcGetPatternData(intersection_data, params).width == 1.f); //Half sized weft, remember width is defined to be across.
    assert(wcGetPatternData(intersection_data, params).length == 3.f); //1.5*2 sized weft because of extension...
}

static void test_sampling_between_yarns_returns_default_yarn_diffuse_color() {
    wcWeaveParameters *params = &params_halfsize;
    //default yarn color should be returned from diffuse if sample between yarns.
    
    intersection_data.uv_x = 0.1;
    intersection_data.uv_y = 0.1;
    wcPatternData pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.yarn_hit == 0);
    wcColor color = wcEvalDiffuse(intersection_data,pattern_data,params);
    wcColor color_expected;
    
    color_expected.r = params->pattern->yarn_types[0].color[0];
    color_expected.g = params->pattern->yarn_types[0].color[1];
    color_expected.b = params->pattern->yarn_types[0].color[2];

    //printf("c_expected: %f %f %f; c: %f %f %f \n", color_expected.r,
    //        color_expected.g, color_expected.b, color.r, color.g, color.b);

    assert(color.r == color_expected.r);
    assert(color.g == color_expected.g);
    assert(color.b == color_expected.b);
}

//TODO
static void test_extending_warp_segments_at_weave_pattern_border() {
    //see plain2x2.png for reference.

    wcWeaveParameters *params = &params_halfsize;
    assert(params->pattern->yarn_types[0].yarnsize == 1.f);
    assert(params->pattern->yarn_types[1].yarnsize == 0.5);
    assert(params->pattern->yarn_types[2].yarnsize == 0.5);
    
    //x = 0.25
    intersection_data.uv_x = 0.25; 
    intersection_data.uv_y = 0.25 - 0.126;
    wcPatternData pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.width == 1.f);
    assert(pattern_data.length == 3.f);
    assert(pattern_data.warp_above == 1);
    assert(pattern_data.x == 0.f);
    assert(pattern_data.y > 0.9 && pattern_data.y <= 1.0); //should be at end of segment

    intersection_data.uv_x = 0.25;
    intersection_data.uv_y = 0.75 - 0.25 - 0.124;
    pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.warp_above == 1);
    assert(pattern_data.x == 0.f);
    assert(pattern_data.y < -0.9 && pattern_data.y >= -1.f); //should be at beginning of segment

    intersection_data.uv_x = 0.25;
    intersection_data.uv_y = 0.75;
    pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.width == 1.f);
    assert(pattern_data.length == 3.f);
    assert(pattern_data.warp_above == 1);
    assert(pattern_data.x == 0.f);
    assert(pattern_data.y == 0.f); //Should be att middle of segment
    
    intersection_data.uv_x = 0.25;
    intersection_data.uv_y = 0.75 + 0.24;
    pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.width == 1.f);
    assert(pattern_data.length == 3.f);
    assert(pattern_data.warp_above == 1);
    assert(pattern_data.x == 0.f);
    assert(pattern_data.y < 0.69 && pattern_data.y >= 0.6); //should be 2/3 along segment

    intersection_data.uv_x = 0.25;
    intersection_data.uv_y = 0.75 + 0.25 + 0.124; //UV WRAPPING OCCURS!
    pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.width == 1.f);
    assert(pattern_data.length == 3.f);
    assert(pattern_data.warp_above == 1);
    assert(pattern_data.x == 0.f);
    assert(pattern_data.y > 0.9 && pattern_data.y <= 1.0); //should be at end of segment
    
    //x=0.75
    intersection_data.uv_x = 0.75; 
    intersection_data.uv_y = 0.75 + 0.126;
    pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.width == 1.f);
    assert(pattern_data.length == 3.f);
    assert(pattern_data.warp_above == 1);
    assert(pattern_data.x == 0.f);
    assert(pattern_data.y < -0.9 && pattern_data.y >= -1.f); //should be at beginning of segment
}

static void test_extending_weft_segments_at_weave_pattern_border() {
    //see plain2x2.png for reference.

    wcWeaveParameters *params = &params_halfsize;
    assert(params->pattern->yarn_types[0].yarnsize == 1.f);
    assert(params->pattern->yarn_types[1].yarnsize == 0.5);
    assert(params->pattern->yarn_types[2].yarnsize == 0.5);

    //Note! returned x and y from pattern_data
    //are defined so that y always is along the yarn...
    
    //uv_y = 0.25 
    intersection_data.uv_y = 0.25; 
    intersection_data.uv_x = 0.25 - 0.25 - 0.124; //UV WRAPPING!
    wcPatternData pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.width == 1.f);
    assert(pattern_data.length == 3.f);
    assert(pattern_data.warp_above == 0);
    assert(pattern_data.x == 0.f);
    assert(pattern_data.y < -0.9 && pattern_data.y >= -1.f); //should be at beginning of segment

    intersection_data.uv_y = 0.25;
    intersection_data.uv_x = 0.25;
    pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.width == 1.f);
    assert(pattern_data.length == 3.f);
    assert(pattern_data.warp_above == 0);
    assert(pattern_data.x == 0.f);
    assert(pattern_data.y == 0.f); //Should be att middle of segment
    
    intersection_data.uv_y = 0.25;
    intersection_data.uv_x = 0.25 + 0.24;
    pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.width == 1.f);
    assert(pattern_data.length == 3.f);
    assert(pattern_data.warp_above == 0);
    assert(pattern_data.x == 0.f);
    assert(pattern_data.y < 0.69 && pattern_data.y >= 0.6); //should be 2/3 along segment

    intersection_data.uv_y = 0.25;
    intersection_data.uv_x = 0.25 + 0.25 + 0.124;
    pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.width == 1.f);
    assert(pattern_data.length == 3.f);
    assert(pattern_data.warp_above == 0);
    assert(pattern_data.x == 0.f);
    assert(pattern_data.y > 0.9 && pattern_data.y <= 1.0); //should be at end of segment
    
    //extra end
    intersection_data.uv_y = 0.25;
    intersection_data.uv_x = 0.75 + 0.126;
    pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.width == 1.f);
    assert(pattern_data.length == 3.f);
    assert(pattern_data.warp_above == 0);
    assert(pattern_data.x == 0.f);
    assert(pattern_data.y < -0.9 && pattern_data.y >= -1.f); //should be at beginning of segment
    
    //uv_y=0.75
    intersection_data.uv_y = 0.75; 
    intersection_data.uv_x = 0.25 - 0.126;
    pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.width == 1.f);
    assert(pattern_data.length == 3.f);
    assert(pattern_data.warp_above == 0);
    assert(pattern_data.x == 0.f);
    assert(pattern_data.y > 0.9 && pattern_data.y <= 1.0); //should be at end of segment
}

static void test_extending_with_all_parallell_warps () {
    wcWeaveParameters params_2parallell;
    wcWeaveParameters *params;
    params = &params_2parallell;
    params->realworld_uv = false;
    params->uscale = params->vscale = 1.f;
    wcWeavePatternFromFile(params,"3parallellwarps.wif");
    params->pattern->yarn_types[1].yarnsize = 0.5;
    params->pattern->yarn_types[2].yarnsize = 0.5;
    wcFinalizeWeaveParameters(params);
    /*
    for(uint32_t y = 0; y < params->pattern_height; y++){
        printf("[ ");
        for(uint32_t x = 0; x < params->pattern_width; x++){
            printf("%d",
                params->pattern->entries[x+y*params->pattern_width].warp_above);
        }
        printf(" ]\n");
    }*/
    wcPatternData pattern_data;

    //testing
    assert(params->pattern->yarn_types[0].yarnsize == 1.f);
    assert(params->pattern->yarn_types[1].yarnsize == 0.5);
    assert(params->pattern->yarn_types[2].yarnsize == 0.5);

    //all parallell warps
    intersection_data.uv_y = 1.f/6.f;
    intersection_data.uv_x = 1.f/6.f;// + (0.5/2.f)/3.f + 0.01;
    pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.width == 1.f);
    assert(pattern_data.length == 5.f); //(2+0.5)*2
    assert(pattern_data.warp_above == 1);
    assert(pattern_data.yarn_hit == 1);

    //When hitting between all parallell warps, return default yarn background.
    intersection_data.uv_y = 1.f/6.f;
    intersection_data.uv_x = 1.f/6.f + (0.5/2.f)/3.f + 0.01;
    pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.yarn_hit == 0);
    /*printf("uv_x: %f, uv_y: %f \n", intersection_data.uv_x, intersection_data.uv_y);
    printf("x: %f, y: %f \n", pattern_data.x, pattern_data.y);
    printf("w: %f, l: %f \n", pattern_data.width, pattern_data.length);
    printf("warp_above: %d, yarn_hit: %d, yarn_type: %d \n", pattern_data.warp_above, pattern_data.yarn_hit, pattern_data.yarn_type);
    */
}

static void test_extending_with_all_parallell_wefts () {
    wcWeaveParameters params_2parallell;
    wcWeaveParameters *params;
    params = &params_2parallell;
    params->realworld_uv = false;
    params->uscale = params->vscale = 1.f;
    wcWeavePatternFromFile(params,"3parallellwefts.wif");
    params->pattern->yarn_types[1].yarnsize = 0.5;
    params->pattern->yarn_types[2].yarnsize = 0.5;
    wcFinalizeWeaveParameters(params);
    wcPatternData pattern_data;

    //testing
    assert(params->pattern->yarn_types[0].yarnsize == 1.f);
    assert(params->pattern->yarn_types[1].yarnsize == 0.5);
    assert(params->pattern->yarn_types[2].yarnsize == 0.5);

    //all parallell wefts
    intersection_data.uv_y = 1.f/6.f;
    intersection_data.uv_x = 3.f/6.f;
    pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.width == 1.f);
    assert(pattern_data.length == 3.f); //(1 + 2*0.25)*2
    assert(pattern_data.warp_above == 0);
    assert(pattern_data.yarn_hit == 1);

    //When hitting between all parallell wefts, return default yarn background.
    intersection_data.uv_y = 1.f/6.f + (0.5/2.f)/3.f + 0.01;
    intersection_data.uv_x = 3.f/6.f;
    pattern_data = wcGetPatternData(intersection_data, params);
    assert(pattern_data.yarn_hit == 0);
}

//TODO
static void test_extended_segments_between_two_parallel_yarns_have_no_specular () {
    //Thinking is that visible segments that are between two parallell thin warps 
    //will not bend. Should therefore not have specular highlight.
}

static void setup() {
    intersection_data.wi_z = 1.f;
    intersection_data.context = NULL;

    printf("Setup fullsize pattern... \n");
    wcWeaveParameters *params = &params_fullsize;
    params->realworld_uv = false;
    params->uscale = params->vscale = 1.f;
    printf("Params uscale: %f \n", params->uscale);
    printf("Params vscale: %f \n", params->vscale);
    wcWeavePatternFromFile(params,"54235plain.wif");
    printf("Pattern height: %d \n", params->pattern_height);
    printf("Pattern width: %d \n", params->pattern_width);
    printf("Pattern realheight: %f \n", params->pattern_realheight);
    printf("Pattern realwidth: %f \n", params->pattern_realwidth);
    printf("Pattern:\n");

    for(uint32_t y = 0; y < params->pattern_height; y++){
        printf("[ ");
        for(uint32_t x = 0; x < params->pattern_width; x++){
            printf("%d",
                params->pattern->entries[x+y*params->pattern_width].yarn_type);
        }
        printf(" ]\n");
    }
    printf("(numbers in pattern indicate yarn type) \n \n");
    printf("Yarn types: \n");
    for(uint32_t i = 0; i < params->pattern->num_yarn_types; i++){
        printf("%d: \n",i);
        printf("    yarnsize: %f \n", params->pattern->yarn_types[i].yarnsize);
        printf("    spec_strength: %f \n", params->pattern->yarn_types[i].specular_strength);
    }
    printf("\n");
    
    printf("Setup halfsize pattern... \n");
    params = &params_halfsize;
    params->realworld_uv = false;
    params->uscale = params->vscale = 1.f;
    printf("Params uscale: %f \n", params->uscale);
    printf("Params vscale: %f \n", params->vscale);
    wcWeavePatternFromFile(params,"54235plain.wif");
    printf("Pattern height: %d \n", params->pattern_height);
    printf("Pattern width: %d \n", params->pattern_width);
    printf("Pattern realheight: %f \n", params->pattern_realheight);
    printf("Pattern realwidth: %f \n", params->pattern_realwidth);
    printf("Pattern:\n");
    printf("Pattern:\n");

    params->pattern->yarn_types[1].yarnsize = 0.5;
    params->pattern->yarn_types[2].yarnsize = 0.5;

    //Somewhat unique default diffuse color.
    params->pattern->yarn_types[0].color[0] = 0.5;
    params->pattern->yarn_types[0].color[1] = 0.0;
    params->pattern->yarn_types[0].color[2] = 0.5;

    wcFinalizeWeaveParameters(params);

    for(uint32_t y = 0; y < params->pattern_height; y++){
        printf("[ ");
        for(uint32_t x = 0; x < params->pattern_width; x++){
            printf("%d",
                params->pattern->entries[x+y*params->pattern_width].yarn_type);
        }
        printf(" ]\n");
    }
    printf("(numbers in pattern indicate yarn type) \n \n");
    printf("Yarn types: \n");
    for(uint32_t i = 0; i < params->pattern->num_yarn_types; i++){
        printf("%d: \n",i);
        printf("    yarnsize: %f \n", params->pattern->yarn_types[i].yarnsize);
        printf("    spec_strength: %f \n", params->pattern->yarn_types[i].specular_strength);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    setup();

    test(sample_yarntype_with_uv);
    test(sampling_with_thin_weft_and_warp);
    test(sampling_between_yarns_returns_default_yarn_diffuse_color);
    test(extending_warp_segments_at_weave_pattern_border);
    test(extending_weft_segments_at_weave_pattern_border);
    test(extending_with_all_parallell_warps);
    test(extending_with_all_parallell_wefts);
}



//Define dummy wceval for texmaps
float wc_eval_texmap_mono(void *texmap, void *data)
{
	return 1.f;
}
void wc_eval_texmap_color(void *texmap, void *data, float *col)
{
    col[0]=1.f; col[1]=1.f; col[2]=1.f;
}
