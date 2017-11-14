#version 330 core

// Output data
out float fragmentdepth;

void main()
{
    // Not really needed, OpenGL does it anyway
    fragmentdepth = gl_FragCoord.z;
}