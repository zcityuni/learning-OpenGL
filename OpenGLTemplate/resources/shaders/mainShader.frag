#version 400 core

in vec3 vColour;			// Interpolated colour using colour calculated in the vertex shader
in vec2 vTexCoord;			// Interpolated texture coordinate using texture coordinate from the vertex shader

out vec4 vOutputColour;		// The output colour

uniform sampler2D sampler0;  // The texture sampler
uniform samplerCube CubeMapTex;
uniform bool bUseTexture;    // A flag indicating if texture-mapping should be applied
uniform bool renderSkybox;
in vec3 worldPosition;


void main()
{


	if (renderSkybox) {
        vOutputColour = texture(CubeMapTex, worldPosition);
    } else {
        // Get the texel colour from the texture sampler
        vec4 vTexColour = texture(sampler0, vTexCoord);

        if (bUseTexture)
            // Use the raw texture color
            vOutputColour = vTexColour;  
        else
            // Or just the vertex colour if texture is off
            vOutputColour = vec4(vColour, 1.0f);
    }

	
	
}
