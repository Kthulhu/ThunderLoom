//More Vray specific things. Implements methods for the BSDFSampler interface.

#include "dynamic.h"
#include "vraybase.h"
#include "vrayinterface.h"
#include "vraycore.h"
#include "vrayrenderer.h"
#include "imtl.h"
#include "helper.h"

#include "VrayThunderLoomBRDF.h"

#ifdef DYNAMIC
// These eval functions are loaded in vrayblinn.cpp when ui is initialized
// and gets used for the the eval method of the MyBaseBSDF class.
extern EVALSPECULARFUNC EvalSpecularFunc;
extern EVALDIFFUSEFUNC  EvalDiffuseFunc;
#endif

using namespace VUtils;

//NOTE(Vidar): This function is called for each intersection with the material
// From this point, several rays can be fired in different directions, each
// one calling eval(). In this function we can do all the work that is common
// throughout all directions, such as computing the diffuse color.
void
MyBaseBSDF::init(const VRayContext &rc, wcWeaveParameters *weave_parameters) {
    m_weave_parameters = weave_parameters;
    EvalDiffuseFunc(rc,weave_parameters,&diffuse_color,&m_yarn_type);
    orig_backside = rc.rayresult.realBack;

	// Set the normals to use for lighting
	normal=rc.rayresult.normal;
	gnormal=rc.rayresult.gnormal;
	if (rc.rayresult.realBack) {
		normal=-normal;
		gnormal=-gnormal;
	}
    // Compute the normal matrix
    if (rc.rayparams.viewDir*normal>0.0f){
		makeNormalMatrix(gnormal, nm);
    } else {
		makeNormalMatrix( normal, nm);
    }
    inm=inverse(nm);
}

// From BRDFSampler

//NOTE(Vidar): Return the normal used for diffuse shading. If we do bump mapping
// on the diffuse color, we will have to return the modified normal here...
VUtils::Vector MyBaseBSDF::getDiffuseNormal(const VR::VRayContext &rc)
{
    return rc.rayresult.origNormal;
}
VUtils::Color MyBaseBSDF::getDiffuseColor(VUtils::Color &lightColor) {
    VUtils::Color ret = diffuse_color*lightColor;
	lightColor.makeZero();
    return ret;
}
VUtils::Color MyBaseBSDF::getLightMult(VUtils::Color &lightColor) {
    float s = m_yarn_type.specular_strength;
    if(!m_yarn_type.specular_strength_enabled){
        s = m_weave_parameters->yarn_types[0].specular_strength;
    }
    VUtils::Color ret = (diffuse_color + VUtils::Color(s,s,s)) * lightColor;
    lightColor.makeZero();
    return ret;
}
VUtils::Color MyBaseBSDF::getTransparency(const VRayContext &rc) {
	return VUtils::Color(0.f,0.f,0.f);
}

VUtils::Color MyBaseBSDF::eval(const VRayContext &rc, const Vector &direction,
    VUtils::Color &lightColor, VUtils::Color &origLightColor, float probLight,
    int flags) {

    VUtils::Color ret(0.f,0.f,0.f);

    float cs = direction * rc.rayresult.normal;
    cs = cs < 0.0f ? 0.0f : cs;

    if(flags & FBRDF_DIFFUSE){
        float k = cs;
        // Apply combined sampling ONLY if GI is on which will pick up the rest
        // of the result
        if (rc.rayparams.localRayType & RT_IS_GATHERING_POINT){
            float probReflection=k;
            //NOTE(Vidar): Multiple importance sampling factor
            k = getReflectionWeight(probLight, probReflection);
        }
        ret += diffuse_color * k;
    }
    if((flags & FBRDF_SPECULAR) && ((rc.rayparams.rayType & RT_NOSPECULAR) == 0)){
        VUtils::Color reflect_color;
        //TODO(Vidar):Better importance sampling... Cosine weighted for now
        float probReflection=cs;
		EvalSpecularFunc(rc,direction,m_weave_parameters,nm,&reflect_color);

        //NOTE(Vidar): Multiple importance sampling factor
        float weight = getReflectionWeight(probLight,probReflection);
        ret += cs*reflect_color*weight;
    }

    ret *= lightColor;

    lightColor.makeZero();
    origLightColor.makeZero();

    return ret;
}

//NOTE(Vidar): Called whenever a reflection ray is to be calculated
void MyBaseBSDF::traceForward(VRayContext &rc, int doDiffuse) {
	BRDFSampler::traceForward(rc, doDiffuse);
	if (doDiffuse) rc.mtlresult.color+=rc.evalDiffuse()*diffuse_color;
}

int MyBaseBSDF::getNumSamples(const VRayContext &rc, int doDiffuse) {
    if(rc.rayparams.currentPass==RPASS_GI ||
        (rc.rayparams.rayType & RT_LIGHT)!=0) {
        return 0;
    }
    //TODO(Vidar):Select using DMC sampler?
    return 8;
}

VRayContext* MyBaseBSDF::getNewContext(const VRayContext &rc, int &samplerID, int doDiffuse) {
    //doDiffuse == 0 => only specular
    //doDiffuse == 1 => specular & diffuse
    //doDiffuse == 2 => only diffuse
	if (2==doDiffuse) return NULL;

    float s = m_yarn_type.specular_strength;
    if(!m_yarn_type.specular_strength_enabled){
        s = m_weave_parameters->yarn_types[0].specular_strength;
    }
    VUtils::Color reflect_filter = VUtils::Color(s,s,s);
	VRayContext &nrc=rc.newSpawnContext(2, reflect_filter, RT_REFLECT | RT_GLOSSY | RT_ENVIRONMENT, normal);

	// Set up the new context
	nrc.rayparams.dDdx.makeZero(); // Zero out the directional derivatives
	nrc.rayparams.dDdy.makeZero();
	nrc.rayparams.mint=0.0f; // Set the ray extents
	nrc.rayparams.maxt=1e18f;
	nrc.rayparams.tracedRay.p=rc.rayresult.wpoint; // Set the new ray origin to be the surface hit point
	return &nrc;
}

ValidType MyBaseBSDF::setupContext(const VRayContext &rc, VRayContext &nrc, float uc, int doDiffuse) {
    //TODO(Vidar):Better importance sampling... Cosine weighted for now
    Vector dir = getDiffuseDir(uc, BRDFSampler::getDMCParam(nrc,1),nrc.rayparams.rayProbability);
    if (dir*gnormal<0.0f) return false;
    VR::getReflectDerivs(rc.rayparams.viewDir, dir, rc.rayparams.dDdx, rc.rayparams.dDdy, nrc.rayparams.dDdx, nrc.rayparams.dDdy);
    nrc.rayparams.tracedRay.dir=nrc.rayparams.viewDir=dir;
    return true;
}

RenderChannelsInfo* MyBaseBSDF::getRenderChannels(void) { return &RenderChannelsInfo::reflectChannels; }

//NOTE(Vidar):Return the correct BRDF depending on which side of the surface
// was hit...
BRDFSampler* MyBaseBSDF::getBRDF(BSDFSide side) {
    if (side==bsdfSide_front) {
        if (orig_backside) return NULL;
    } else {
        if (!orig_backside) return NULL;
    }
    return static_cast<BRDFSampler*>(this);
}
