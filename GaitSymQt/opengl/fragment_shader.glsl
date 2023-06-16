#version 330 core

out highp vec4 colorOut;

uniform highp vec4 diffuse;
uniform highp vec4 ambient;
uniform highp vec4 specular;
uniform highp float shininess;

uniform highp vec4 blendColour;
uniform highp float blendFraction;

uniform highp bool hasTexture;
uniform highp sampler2D textureSampler;

in highp vec3 normalFrag;
in highp vec3 eyeFrag;
in highp vec3 lightDirFrag;
in highp vec4 colourFrag;
in highp vec2 uvFrag;

void main()
{
    if (hasTexture)
    {
        // just sample the texture - currently no lighting
        colorOut = texture(textureSampler, uvFrag);
    }
    else
    {
        // part 2 of a generic Phong shading model shader with optional blendColour
        vec4 blendColourFrag = blendColour * blendFraction + colourFrag * (1.0 - blendFraction);

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
        // end of Phong section
    }
}



