// Dynamic.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "dynamic.h"
#include "dbgprint.h"
#include "vraybase.h"
#include "vrayinterface.h"
#include "vraycore.h"
#include "vrayrenderer.h"
#include "shadedata_new.h"
#include "woven_cloth.h"
#include "../3dsMax/helper.h"

#define _USE_MATH_DEFINES
#include <math.h>

void
#ifdef DYNAMIC
eval_diffuse
#else
EvalDiffuseFunc
#endif
(const VUtils::VRayContext &rc,
    wcWeaveParameters *weave_parameters, Texmap **texmaps,
	VUtils::Color *diffuse_color, YarnType* yarn_type)
{
    if(weave_parameters->pattern == 0){ //Invalid pattern
        *diffuse_color = VUtils::Color(1.f,1.f,0.f);
		*yarn_type = default_yarn_type;
        return;
    }
    wcIntersectionData intersection_data;
   
    const VR::VRayInterface &vri_const=static_cast<const VR::VRayInterface&>(rc);
	VR::VRayInterface &vri=const_cast<VR::VRayInterface&>(vri_const);
	ShadeContext &sc=static_cast<ShadeContext&>(vri);

    Point3 uv = sc.UVW(1);

    intersection_data.uv_x = uv.x;
    intersection_data.uv_y = uv.y;
    intersection_data.wi_z = 1.f;

    wcPatternData pattern_data = wcGetPatternData(intersection_data,
        weave_parameters);
	*yarn_type = weave_parameters->pattern->yarn_types[pattern_data.yarn_type];
	float specular_strength = yarn_type->specular_strength;
    wcColor d = 
        wcEvalDiffuse( intersection_data, pattern_data, weave_parameters);

	/*
	//For texmap variation this is correct but does not look good! causes sudden changes in diffuse color...
	//If there is more specular, there should be less diffuse and vice versa.
	int offset = NUMBER_OF_FIXED_TEXMAPS + pattern_data.yarn_type*NUMBER_OF_YRN_TEXMAPS;
	float specular_variation = 1.f;
	if (texmaps[offset + yrn_texmaps_specular]) {
		specular_variation = texmaps[offset + yrn_texmaps_specular]->EvalMono(sc);
	} else if (texmaps[texmaps_specular]) {
		specular_variation = texmaps[texmaps_specular]->EvalMono(sc);
	}
	specular_strength *= specular_variation; 
	*/

	//Texmap variation
	AColor diffuse_map;
	int offset = NUMBER_OF_FIXED_TEXMAPS + pattern_data.yarn_type*NUMBER_OF_YRN_TEXMAPS;
	if (texmaps[offset + yrn_texmaps_diffuse]) {
		diffuse_map = texmaps[offset + yrn_texmaps_diffuse]->EvalColor(sc);
	} else if (texmaps[texmaps_diffuse]) {
		diffuse_map = texmaps[texmaps_diffuse]->EvalColor(sc);
	} else {
		diffuse_map.White();
	}

	//TODO(Peter): Currently diffuse map multiplies color to result. 
	//Should make this clear in the gui
	//TODO(Peter): Move actual modification into woven_cloth project,
	//pass variation mount in with intersection_data?

	float factor = (1.f - specular_strength);
    diffuse_color->r = factor*diffuse_map.r*d.r;
	diffuse_color->g = factor*diffuse_map.g*d.g;
	diffuse_color->b = factor*diffuse_map.b*d.b;
}

void
#ifdef DYNAMIC
eval_specular
#else
EvalSpecularFunc
#endif
( const VUtils::VRayContext &rc, const VUtils::Vector &direction,
    wcWeaveParameters *weave_parameters, Texmap **texmaps, VUtils::Matrix nm,
    VUtils::Color *reflection_color)
{
    if(weave_parameters->pattern == 0){ //Invalid pattern
		float s = 0.1f;
		reflection_color->r = s;
		reflection_color->g = s;
		reflection_color->b = s;
		return;
    }
    wcIntersectionData intersection_data;
   
    const VR::VRayInterface &vri_const=static_cast<const VR::VRayInterface&>(rc);
	VR::VRayInterface &vri=const_cast<VR::VRayInterface&>(vri_const);
	ShadeContext &sc=static_cast<ShadeContext&>(vri);

    Point3 uv = sc.UVW(1);

    //Convert the view and light directions to the correct coordinate system
    Point3 viewDir, lightDir;
    viewDir.x = -rc.rayparams.viewDir.x;
    viewDir.y = -rc.rayparams.viewDir.y;
    viewDir.z = -rc.rayparams.viewDir.z;
    viewDir = sc.VectorFrom(viewDir,REF_WORLD);
    viewDir = viewDir.Normalize();

    lightDir.x = direction.x;
    lightDir.y = direction.y;
    lightDir.z = direction.z;
    lightDir = sc.VectorFrom(lightDir,REF_WORLD);
    lightDir = lightDir.Normalize();

    // UVW derivatives
    Point3 dpdUVW[3];
    sc.DPdUVW(dpdUVW,1);

    Point3 n_vec = sc.Normal().Normalize();
    Point3 u_vec = dpdUVW[0].Normalize();
    Point3 v_vec = dpdUVW[1].Normalize();
    u_vec = v_vec ^ n_vec;
    v_vec = n_vec ^ u_vec;

    intersection_data.wi_x = DotProd(lightDir, u_vec);
    intersection_data.wi_y = DotProd(lightDir, v_vec);
    intersection_data.wi_z = DotProd(lightDir, n_vec);

    intersection_data.wo_x = DotProd(viewDir, u_vec);
    intersection_data.wo_y = DotProd(viewDir, v_vec);
    intersection_data.wo_z = DotProd(viewDir, n_vec);

    intersection_data.uv_x = uv.x;
    intersection_data.uv_y = uv.y;

    wcPatternData pattern_data = wcGetPatternData(intersection_data,
        weave_parameters);
	float specular_strength = weave_parameters->pattern->
		yarn_types[pattern_data.yarn_type].specular_strength;

	//Texmap variation
	int offset = NUMBER_OF_FIXED_TEXMAPS + pattern_data.yarn_type*NUMBER_OF_YRN_TEXMAPS;
	float specular_variation = 1.f;
	if (texmaps[offset + yrn_texmaps_specular]) {
		specular_variation = texmaps[offset + yrn_texmaps_specular]->EvalMono(sc);
	} else if (texmaps[texmaps_specular]) {
		specular_variation = texmaps[texmaps_specular]->EvalMono(sc);
	}

    float s = specular_strength * specular_variation *
        wcEvalSpecular(intersection_data, pattern_data, weave_parameters);
    reflection_color->r = s;
    reflection_color->g = s;
    reflection_color->b = s;
}