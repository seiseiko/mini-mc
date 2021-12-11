#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments

out float fragmentdepth;

void main()
{
    // Copy the color; there is no shading.
    fragmentdepth = gl_FragCoord.z;
}
