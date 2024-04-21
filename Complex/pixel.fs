#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform int maxIterations;
uniform float time;

// Output fragment color
out vec4 finalColor;

// Modify function and derivative to toy around.
vec2 function(vec2 z);
vec2 derivative(vec2 z);
vec2 cmul(vec2 z, vec2 w);
vec2 cdiv(vec2 z, vec2 w);
vec2 cpow(vec2 z, int n);

void main()
{
    finalColor = vec4(0, 0, 0, 1);
    
    // The roots.
    vec2 roots[3] = {
        vec2(1, 0),
        vec2(-0.5, sqrt(3)/2),
        vec2(-0.5, -sqrt(3)/2)
    };

    // These are the colors for each root.
    vec3 colors[3] = {
        vec3(1, 0, 0),
        vec3(0, 1, 0),
        vec3(0, 0, 1)
    };

    // Linear interpolate to get in the Mandelbrot range of complex numbers.
    float bounds = 2;
    vec2 z;
    z.x = mix(-bounds, bounds, fragTexCoord.x);
    z.y = mix(-bounds, bounds, fragTexCoord.y);

    float tolerance = 0.001;
    int i = 0;
    for ( ; i < maxIterations; i++) {
        z = z - cdiv(function(z), derivative(z));

        bool hasRoot = false;
        for (int j = 0; j < 3; j++) {
            vec2 diff = abs(z - roots[j]);
            if (diff.x < tolerance && diff.y < tolerance) {
                finalColor.rgb = colors[j];
                hasRoot = true;
            }
        }
        
        if (hasRoot) {
            break;
        }
    }
    finalColor.rgb *= 1.0-float(i)/maxIterations;
}

vec2 function(vec2 z)
{
    return cpow(z, 3) - vec2(1, 0);
}

vec2 derivative(vec2 z)
{
    return 3*cmul(z, z);
}

// Utility functions for complex numbers.
// Multiplication, division and integer powers.
vec2 cmul(vec2 z, vec2 w)
{
    return vec2(z.x*w.x-z.y*w.y, z.x*w.y+z.y*w.x);
}

vec2 cdiv(vec2 z, vec2 w)
{
    return cmul(z, vec2(w.x, -w.y)) / dot(w, w);
}

vec2 cpow(vec2 z, int n)
{
    vec2 result = vec2(1, 0);
    for (int i = 0; i < n; i++) {
        result = cmul(result, z);
    }
    return result;
}