ThunderLoom
===
![teaser8](https://cloud.githubusercontent.com/assets/116268/18617674/92d092c4-7dd5-11e6-9e2d-04d64c712c40.png)
*A physically based shader for woven cloth*

This projects consits of three main parts:

* Irawan shading model

At its core is an implementation of the 'Irawan' shading model, as presented in
the [phd thesis](http://www.cs.cornell.edu/~srm/publications/IrawanThesis.pdf)
by Piti Irawan and in the paper by 
[Irawan and Marschner (2012)](http://www.cs.cornell.edu/~srm/publications/TOG12-cloth.html).

* Library with API

The shading model is made accessible for rendering engines with added
flexibilty and artistic control through a library with a simple API. This 
allows the model and extra improvements to quickly be integrated into custom
rendering pipelines or created as plugins to more established renderers.

The library allows fast and flexible control over many parameters of how the
woven cloth is to behave. The actual pattern is determined using standard 
weaving-drafts defined in `WIF` files.
Many types of patterns are freely available from fine sites
such as [handweaving.net](http://handweaving.net).

* Frontends/plugins

Two plugins using this library have been made. Giving two common rendering
pipelines a shader for woven cloth. Currently the 3dsMax/VRay frontend is the
most fully featured. A plugin for Mitsuba has also been created, but this
is lacking many features compared to the 3dsMax frontend, it was first mainly 
created for as a proof of concept of having a core library support multiple renderes. 
See the subfolders for these frontends for
more information. 

## Installing the frontends
Precompiled binaries can be found under the [releases](https://github.com/vidarn/ThunderLoom/releases) tab. The zip file
includes plugins for VRay/3dsMax (2014/2015/2016) and a plugin for the Mitsuba
engine.

### VRay/3dsMax 
(see [frontends/vray3dsMax/README.md](https://github.com/vidarn/ThunderLoom/tree/master/frontends/vray3dsMax) for more info)

Move the `thunderLoom.dlt` file corresponding to your 3dsMax version to the
`vrayplugins` folder at your 3dsMax install location. 

The usual path for the plugins folder is
`C:\Program Files\Autodesk\3dsMax (VERSION)\plugins\vrayplugins`.

### Mitsuba
(see [frontends/mitsuba/README.md](https://github.com/vidarn/ThunderLoom/tree/master/frontends/mitsuba) for more info)

Move the compiled library file to the mitsuba `plugins` directory. 

That is,

* For linux, move `thunderloom_mitsuba.so` into `path-to-mitsuba/plugins`
* For mac, move `thunderloom_mitsuba.dylib` into `/Applications/Mitsuba.app/plugins`
* For windows, move `thunderloom_mitsuba.dll` into `path-to-mitsuba/plugins`

Now the `thunderloom` bsdf should be avaiable the next time you start mitsuba.
See the `example_scenes` folder for examples on how to use the bsdf.

Additionally there is a small patch file available for `mtsblend`
(a blender/mitsuba) plugin which adds UI for the shader in blender. 
See [frontends/mitsuba/blender_mtsblend/README.md](https://github.com/vidarn/ThunderLoom/tree/master/frontends/mitsuba/blender_mtsblend) for more info.

## Library
The source code for the library and core is found in the 'src' folder. 
The file 'woven_cloth.h' describes the API of the shader.

For an example implementaion see [frontends/api_demo/](https://github.com/vidarn/ThunderLoom/tree/master/frontends/api_demo).

## Licence
The source code is released under the MIT Licence.

But if you happen to make something cool using this, a nice render or an 
improved tool, please let us know! We would love to see it! :)
