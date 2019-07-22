#version 330

// this is is a generic Phong shading model shader

out vec4 colorOut;

uniform vec4 diffuse;
uniform vec4 ambient;
uniform vec4 specular;
uniform float shininess;

in vec3 normalFrag;
in vec3 eyeFrag;
in vec3 lightDirFrag;

void main()
{

    vec4 spec = vec4(0.0);

    vec3 n = normalize(normalFrag);
    vec3 l = normalize(lightDirFrag);
    vec3 e = normalize(eyeFrag);

    float intensity = max(dot(n,l), 0.0);
    if (intensity > 0.0)
    {
        vec3 h = normalize(l + e);
        float intSpec = max(dot(h,n), 0.0);
        spec = specular * pow(intSpec, shininess);
    }

    colorOut = max(intensity * diffuse + spec, ambient);
}

