#include "shell-grass.h"

ShellGrass* CreateGrass(u16 widthPixels, u16 heightPixels, u8 shellAmount, float height, float uvRepeat)
{
	ShellGrass* g = (ShellGrass*)mgdl_AllocateGraphicsMemory(sizeof(ShellGrass));
	Random_SetSeed(2350923059);

	GLubyte* wa = (GLubyte*)mgdl_AllocateGeneralMemory(sizeof(GLubyte) * widthPixels * heightPixels * 2);
	GLubyte* wp = wa;
	for(u32 y = 0; y < heightPixels; y++)
	{
		for(u32 x = 0; x < widthPixels; x++)
		{
			(*wp)=255; // White
			wp +=1;
			(*wp)=Random_FloatNormalized() * 255; // random alpha
			wp +=1;
		}
	}
	g->texture = Texture_CreateFromArray(Nearest, widthPixels, heightPixels, wa, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, true);
	g->shellAmount = shellAmount;
	g->uvRepeat = uvRepeat;
	g->height = height;
	return g;
}
void DrawGrass(ShellGrass* grass, color32 color, float minIntensity, float maxIntensity, float size, float ystepoffset)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, grass->texture->textureId);
	float intensityrange = maxIntensity - minIntensity;
	float intensityStep = intensityrange / (float)(grass->shellAmount-1);
	float clipStep = 0.9f / (float)(grass->shellAmount-1);
	// 0.012f is the default step
	static const float defaultYStep = 0.012f;
	float shellYstep = grass->height / (float)(grass->shellAmount-1) + ystepoffset;
	u8 rendersPerShell = 1;
	if (shellYstep > defaultYStep)
	{
		// How many defaultSteps fit into shell step?
		rendersPerShell = floor(shellYstep/defaultYStep);
	}

	//glEnable(GL_ALPHA_TEST);
	RGBAf color3 = Color_HexToFloats(color);

	glPushMatrix();
	// NOTE Draw topmost layer first to avoid overdraw
	float shellY = grass->height;
	for (s8 i = grass->shellAmount-1; i >= 0; i--)
	{
		float treshold = 0.0f + clipStep * i;
		// NOTE Cannot change treshold inside glBegin - glEnd
		glAlphaFunc(GL_GEQUAL, treshold);
		glBegin(GL_QUADS);
			glNormal3f(0.0f, 1.0f, 0.0f);
		// Only draw pixels with alpha greater than treshold
		// Use big treshold on topmost layer
			float intensity = minIntensity + intensityStep * (float)i;
			glColor3f(color3.red * intensity, color3.green * intensity, color3.blue * intensity);

			float quadY = shellY;
			for(u8 render = 0; render < rendersPerShell; render++)
			{
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-size, quadY, size);
				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(size, quadY, size);
				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(size, quadY, -size);
				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-size, quadY, -size);
				quadY -= defaultYStep;
			}
			shellY -= shellYstep;
		glEnd();

	}
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glAlphaFunc(GL_GEQUAL, 0.5f); // Restore default

}

// TODO Just give RGBAf color directly
void DrawGrassOnPolygonBuffer(ShellGrass* grass, color32 color, Vector3 growDirection, float height, float minIntensity, BufferDrawFunction drawFunction, float activeScale)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, grass->texture->textureId);
	float maxIntensity = 1.0f;
	float intensityrange = maxIntensity - minIntensity;
	float intensityStep = intensityrange / (float)(grass->shellAmount-1);
	float clipStep = 0.9f / (float)(grass->shellAmount-1);
	// 0.012f is the default step
	static const float defaultYStep = 0.012f;
	float shellYstep = height / (float)(grass->shellAmount-1);
	u8 rendersPerShell = 1;
	float remainingY = shellYstep;
	if (shellYstep > defaultYStep)
	{
		// How many defaultSteps fit into shell step?
		rendersPerShell = floor(shellYstep/defaultYStep);
		// How much need to move after all the defaultsteps
		remainingY = shellYstep - rendersPerShell * defaultYStep;
	}

	glNormal3f(growDirection.x, growDirection.y, growDirection.z);

	// NOTE Draw topmost layer first to avoid overdraw
	float shellY = height;

	RGBAf color3 = Color_HexToFloats(color);

	// Keep changing this, dont make a big matrix stack
	Vector3 growEnd = Vector3Scale(growDirection, shellY);
	Vector3 growChange = growDirection;
	float antiScale = 1.0f/activeScale;
	// Translate to top of grass
	for (s8 i = grass->shellAmount-1; i >= 0; i--)
	{
		float treshold = 0.0f + clipStep * i;
		// NOTE Cannot change treshold inside glBegin - glEnd
		glAlphaFunc(GL_GEQUAL, treshold);
		float intensity = minIntensity + intensityStep * (float)i;
		glColor3f(color3.red * intensity, color3.green * intensity, color3.blue * intensity);
		// Only draw pixels with alpha greater than treshold
		// Use big treshold on topmost layer
			for(u8 render = 0; render < rendersPerShell; render++)
			{
			glPushMatrix();
				// Negate active scale
				Vector3 scaledEnd = Vector3Scale(growEnd, antiScale);
				glTranslatef(scaledEnd.x, scaledEnd.y, scaledEnd.z);
				drawFunction();
			glPopMatrix();
				// Only translate if need to.
				if (rendersPerShell > 1)
				{
					growChange = Vector3Scale(growDirection, -defaultYStep);
					growEnd = Vector3Add(growEnd, growChange);
				}
			}
			// Move against growdirection
			growChange = Vector3Scale(growDirection, -remainingY);
			growEnd = Vector3Add(growEnd, growChange);
	}
	glDisable(GL_TEXTURE_2D);

	glAlphaFunc(GL_GEQUAL, 0.5f);
}
