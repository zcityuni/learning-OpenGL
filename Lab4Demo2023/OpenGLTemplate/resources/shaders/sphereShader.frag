#version 400 core

in vec3 vColour;			// Note: colour is smoothly interpolated (default)
out vec4 vOutputColour;

void main()
{	
	vOutputColour = vec4(vColour, 1.0);
}
