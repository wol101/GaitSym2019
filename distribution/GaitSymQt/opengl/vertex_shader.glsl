#version 330 core

uniform highp mat4 mvpMatrix;
uniform highp mat4 mvMatrix;
uniform highp mat3 normalMatrix;

uniform highp vec4 lightPosition;

uniform highp float decal;

in highp vec4 vertex;
in highp vec3 vertexNormal;
in highp vec4 vertexColour;
in highp vec2 vertexUV;

out highp vec3 normalFrag;
out highp vec3 eyeFrag;
out highp vec3 lightDirFrag;
out highp vec4 colourFrag;
out highp vec2 uvFrag;

void main ()
{
    vec4 pos = mvMatrix * vertex;

    // this is part 1 of a generic Phong shading model shader
    normalFrag = normalize(normalMatrix * vertexNormal);
    lightDirFrag = vec3(lightPosition - pos);
    eyeFrag = vec3(-pos);
    colourFrag = vertexColour;
    // end of Phong section

    // 24 bit depth buffer should have a resolution of 2/2^24 which is 1.1920928955078125e-07
    // and FLT_EPSILON is typically 1.192092896e-07
    // x2 flickers occasionally so try x4
    vec4 preDecalPos = mvpMatrix * vertex;
    preDecalPos.z -= decal * 4.768371584e-07;
    gl_Position = preDecalPos;

    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    uvFrag = vertexUV;
}

