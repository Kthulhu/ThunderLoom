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
    
    //Adjascent weft is extended when warp is missed. 
    intersection_data.uv_x = 0.75 - 0.2; //(middle of warp segment, outside left of warp segment)
    intersection_data.uv_y = 0.25;
    assert(wcGetPatternData(intersection_data, params).yarn_hit == 1);
    assert(wcGetPatternData(intersection_data, params).yarn_type == 2);
    assert(wcGetPatternData(intersection_data, params).warp_above == 0);
    
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
static void test_extending_segments_at_weave_pattern_border() {
    wcWeaveParameters *params = &params_halfsize;
    //default yarn color should be returned from diffuse if sample between yarns.
    
    intersection_data.uv_x = 0.25; //(middle of weft segment, outside top of weft segment)
    intersection_data.uv_y = 0.25 + 0.2;
    wcPatternData pattern_data = wcGetPatternData(intersection_data, params);

    //extended segment should be a warp.
    assert(pattern_data.yarn_hit == 1);
    assert(pattern_data.warp_above == 1);
    //assert(pattern_data.length == 2);
    printf("w: %f, l: %f \n", pattern_data.width, pattern_data.length);
    //NOTE(Peter): Getting l=3... is this correct? Shouldn't it be 2?
    //1 full warp cell + 2 extensions, both with length 0.5.
    //Do not see what is wrong in the code though...
}

//TODO
static void test_extending_with_two_parallel_warps () {
    //Test extending when missing a warp and the adjascent cell in pattern matrix also is a warp.
}

//TODO
static void test_extending_with_all_parallell_warps () {
    //Test extending when missing a warp and all adjascent cells in pattern matrix are also warp.
    //What to do then?!?!?!?!?!
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
    params_halfsize = params_fullsize;
    params = &params_halfsize;
    printf("(same as fullsize...)\n");
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
    test(extending_segments_at_weave_pattern_border);
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
