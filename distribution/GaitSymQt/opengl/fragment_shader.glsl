#version 330

// this is is a generic Phong shading model shader

out vec4 colorOut;

uniform vec4 diffuse;
uniform vec4 ambient;
uniform vec4 specular;
uniform float shininess;

uniform vec4 blendColour;
uniform float blendFraction;

in vec3 normalFrag;
in vec3 eyeFrag;
in vec3 lightDirFrag;
in vec4 colourFrag;

void main()
{
    vec4 blendColourFrag = blendColour * blendFraction + colourFrag * (1 - blendFraction);

    vec4 spec = vec4(0.0);

    vec3 n = normalize(normalFrag);
    vec3 l = normalize(lightDirFrag);
    vec3 e = normalize(eyeFrag);

    float intensity = max(dot(n,l), 0.0);
    if (intensity > 0.0)
    {
        vec3 h = normalize(l + e);
        float intSpec = max(dot(h,n), 0.0);
        spec = specular * blendColourFrag * pow(intSpec, shininess);
    }

    colorOut = max(intensity * diffuse * blendColourFrag + spec, ambient * blendColourFrag);
}



