#version 400 core

in vec3 vColour;			// Note: colour is smoothly interpolated (default)
out vec4 vOutputColour;
in float fIntensity;


vec3 toonColour(float fIntensity)
{
		vec3 finalColour;

		// color vectors are in RGB
		vec3 brightGreen = vec3(0.0, 1.0, 0.0); 
		vec3 mediumGreen = vec3(0.0, 0.75, 0.0);
		vec3 darkGreen = vec3(0.0, 0.5, 0.0); 

		if(fIntensity < 0.5){
			finalColour = darkGreen;
		} else if(fIntensity < 0.75){
			finalColour = mediumGreen;
		} else{
			finalColour = brightGreen;
		}
		return finalColour;
}

void main()
{	
	//vOutputColour = vec4(vColour, 1.0);
	vOutputColour = vec4(toonColour(fIntensity), 1.0);
}
