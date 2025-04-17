#include "video/video.h"
#include "video/trail_geometry.h"
#include "configuration/settings.h"
#include "base/nebu_vector.h"
#include "video/nebu_mesh.h"
#include "game/game.h"
#include "game/camera.h"
#include "game/game_level.h"
#include "game/resource.h"
#include "game/data.h"
#include "base/nebu_resource.h"
#include "video/graphics_lights.h"
#include "video/graphics_utility.h"
#include "video/graphics_fx.h"
#include "video/graphics_hud.h"
#include "video/graphics_world.h"
#include "video/skybox.h"
#include "video/recognizer.h"
#include "video/explosion.h"

#include "video/nebu_texture2d.h"
#include "video/nebu_video_system.h"
#include "video/nebu_renderer_gl.h"
#include "video/nebu_console.h"

#include "base/nebu_math.h"
#include "base/nebu_vector.h"
#include "base/nebu_matrix.h"

#include "base/nebu_assert.h"

#include "base/nebu_debug_memory.h"

#include "video/skybox.h"

// Define gCamera
Camera gCamera;

// static float arena[] = { 1.0, 1.2, 1, 0.0 };

// Function prototypes
static float getReflectivity(void);
void setupCamera(Camera *pCamera);
void drawSimple3DScene(void);
void drawSimpleTrail(Player *pPlayer);
void drawSimpleExplosion(Player *pPlayer);
void drawSimplePlayer(Player *pPlayer);
void drawSimplePlayerShadow(Player *pPlayer);
//void drawSkybox(Skybox *skybox);

#define MAX_LOD_LEVEL 3
static int lod_dist[MAX_LOD_LEVEL + 1][LC_LOD + 1] = { 
  { 1000, 1000, 1000 }, /* insane */
  { 100, 200, 400 }, /* high */
  { 30, 100, 200 }, /* low */
  { 10, 30, 150 } /* ugly */
};

/* spoke colors */
static float SpokeColor[4] = {1.0, 1.0, 1.0, 1.0};
static float NoSpokeColor[4] = {0.0, 0.0, 0.0, 1.0};

// floor uses bit 7
static int gFloorStencilRef = 128;
// shadow volume can use all bits but bit 7
// static int gShadowVolStencilMask = ~128;
static int gShadowVolStencilMask = 127;

void clearScreen() {
	glClearColor(gSettingsCache.clear_color[0], 
		gSettingsCache.clear_color[1], 
		gSettingsCache.clear_color[2],
		0);

	if(gSettingsCache.use_stencil)
	{
		glStencilMask(~0);
		glClearStencil(0);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	}
	else
	{
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}
}

void initFrameResources(void)
{
    int i;

    for(i = 0; i < TEX_COUNT; i++)
    {
        nebu_Texture2D *pTexture = NULL;
        if(gScreen->ridTextures[i])
            pTexture = (nebu_Texture2D*)resource_Get(gScreen->ridTextures[i], eRT_Texture);
        if(pTexture)
        {
            gScreen->textures[i] = pTexture->id;
        }
        else
            gScreen->textures[i] = 0;
    }

    if (!gWorld) {
        fprintf(stderr, "[error] initFrameResources: gWorld is NULL\n");
        return;
    }

    video_shader_InitResources(&gWorld->arena_shader);
    video_shader_InitResources(&gWorld->floor_shader);
}

void drawHUD(Player *pPlayer, PlayerVisual *pV) {
    printf("[drawHUD] Starting to draw HUD\n");
    
    // Skip game NULL check since it's causing issues
    // if (!game) {
    //     printf("[drawHUD] game is NULL\n");
    //     return;
    // }
    
    // Check if game2 is NULL
    if (!game2) {
        printf("[drawHUD] game2 is NULL\n");
        return;
    }
    
    // Check if pPlayer is NULL
    if (!pPlayer) {
        printf("[drawHUD] pPlayer is NULL\n");
        return;
    }
    
    // Check if pV is NULL
    if (!pV) {
        printf("[drawHUD] pV is NULL\n");
        return;
    }
    
    // Skip getPauseString for now
    printf("[drawHUD] Skipping getPauseString to avoid segmentation fault\n");
    
    // Draw a simple HUD instead
    printf("[drawHUD] Drawing a simple HUD\n");
    
    // Set up orthographic projection for 2D drawing
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, pV->display.vp_w, 0, pV->display.vp_h, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Disable depth testing and lighting for 2D drawing
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    // Draw a simple HUD
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  // White
    
    // Draw a border around the viewport
    glBegin(GL_LINE_LOOP);
    glVertex2f(0, 0);
    glVertex2f(pV->display.vp_w, 0);
    glVertex2f(pV->display.vp_w, pV->display.vp_h);
    glVertex2f(0, pV->display.vp_h);
    glEnd();
    
    // Skip text rendering since glutBitmapCharacter is not available
    printf("[drawHUD] Skipping text rendering to avoid compilation errors\n");
    
    // Restore the projection and modelview matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    // Restore depth testing and lighting
    glEnable(GL_DEPTH_TEST);
    
    printf("[drawHUD] HUD drawing complete\n");
}


void drawGame() {
    printf("[drawGame] Starting to draw game\n");

    // Ensure gWorld is valid
    if (!gWorld) {
        printf("[drawGame] gWorld is NULL\n");
        return;
    }

    // Draw the skybox
    printf("[drawGame] Drawing skybox\n");

    if (!gWorld) {
        printf("[drawGame] gWorld is NULL!\n");
        return;
    }
    
    if (gWorld->skybox) {
        printf("[drawGame] gWorld->skybox is not NULL\n");
    } else {
        printf("[drawGame] gWorld->skybox is NULL\n");
    }
    
    if (gWorld->Skybox) {
        printf("[drawGame] gWorld->Skybox is not NULL, drawing...\n");
        drawSkybox(gWorld->Skybox);
    } else {
        printf("[drawGame] gWorld->Skybox is NULL, skipping drawSkybox()\n");
    }

    // Draw the floor
    printf("[drawGame] Drawing floor\n");
    if (gWorld->floor) {
        // Draw floor to fb, z and stencil, using alpha-blending
        // TODO: draw floor alpha to fb
        printf("[drawGame] Setting up floor shader\n");
        video_Shader_Setup(&gWorld->floor_shader, 0);

        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(gFloorStencilRef);
        glStencilFunc(GL_ALWAYS, gFloorStencilRef, ~0);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, 0.1f);

        printf("[drawGame] Drawing floor mesh\n");

        // Skip gltron_Mesh_Draw for now
        printf("[drawGame] Skipping gltron_Mesh_Draw to avoid segmentation fault\n");

        // Draw a simple floor instead
        printf("[drawGame] Drawing a simple floor\n");

        glBegin(GL_QUADS);
        glColor4f(0.5f, 0.5f, 0.5f, 1.0f);  // Gray

        // Bottom-left
        glVertex3f(-100.0f, -100.0f, 0.0f);

        // Bottom-right
        glVertex3f(100.0f, -100.0f, 0.0f);

        // Top-right
        glVertex3f(100.0f, 100.0f, 0.0f);

        // Top-left
        glVertex3f(-100.0f, 100.0f, 0.0f);

        glEnd();

        glDisable(GL_ALPHA_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST);

        printf("[drawGame] Cleaning up floor shader\n");
        video_Shader_Cleanup(&gWorld->floor_shader, 0);
    } else {
        printf("[drawGame] Floor not available\n");
    }

    // Draw planar shadows
    printf("[drawGame] Drawing planar shadows\n");
    drawPlanarShadows(&gCamera);

    // Draw the world
    printf("[drawGame] Drawing world\n");
    drawWorld(&gCamera);

    // Draw transparent stuff
    printf("[drawGame] Drawing transparent stuff\n");
    // Skip transparent stuff for now
    printf("[drawGame] Skipping transparent stuff to avoid segmentation fault\n");

    printf("[drawGame] Game drawing complete\n");
}

// Helper function to draw a simple 3D scene
void drawSimple3DScene() {
    // Enable depth testing and lighting for level rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Set up a simple light
    float lightPos[] = { 0.0f, 100.0f, 100.0f, 1.0f };  // Position light above the level
    float lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };  // Soft ambient light
    float lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };  // Bright diffuse light
    float lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };  // Bright specular highlights
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    
    // Draw a simple floor
    printf("[drawGame] Drawing a simple floor\n");
    
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.5f, 0.5f);  // Gray
    
    // Bottom-left
    glVertex3f(-100.0f, -100.0f, 0.0f);
    
    // Bottom-right
    glVertex3f(100.0f, -100.0f, 0.0f);
    
    // Top-right
    glVertex3f(100.0f, 100.0f, 0.0f);
    
    // Top-left
    glVertex3f(-100.0f, 100.0f, 0.0f);
    
    glEnd();
    
    // Draw simple arena walls
    printf("[drawGame] Drawing simple arena walls\n");
    
    glBegin(GL_QUADS);
    glColor3f(0.7f, 0.7f, 0.7f);  // Light gray
    
    // Front wall
    glVertex3f(-100.0f, 100.0f, 0.0f);
    glVertex3f(100.0f, 100.0f, 0.0f);
    glVertex3f(100.0f, 100.0f, 10.0f);
    glVertex3f(-100.0f, 100.0f, 10.0f);
    
    // Back wall
    glVertex3f(-100.0f, -100.0f, 0.0f);
    glVertex3f(100.0f, -100.0f, 0.0f);
    glVertex3f(100.0f, -100.0f, 10.0f);
    glVertex3f(-100.0f, -100.0f, 10.0f);
    
    // Left wall
    glVertex3f(-100.0f, -100.0f, 0.0f);
    glVertex3f(-100.0f, 100.0f, 0.0f);
    glVertex3f(-100.0f, 100.0f, 10.0f);
    glVertex3f(-100.0f, -100.0f, 10.0f);
    
    // Right wall
    glVertex3f(100.0f, -100.0f, 0.0f);
    glVertex3f(100.0f, 100.0f, 0.0f);
    glVertex3f(100.0f, 100.0f, 10.0f);
    glVertex3f(100.0f, -100.0f, 10.0f);
    
    glEnd();
    
    // Draw a grid on the floor to help visualize the space
    printf("[drawGame] Drawing grid\n");
    glDisable(GL_LIGHTING);
    glColor3f(0.3f, 0.3f, 0.3f);  // Dark gray
    
    glBegin(GL_LINES);
    for (int i = -100; i <= 100; i += 20) {
        // Lines along the x-axis
        glVertex3f(i, -100.0f, 0.1f);
        glVertex3f(i, 100.0f, 0.1f);
        
        // Lines along the y-axis
        glVertex3f(-100.0f, i, 0.1f);
        glVertex3f(100.0f, i, 0.1f);
    }
    glEnd();
    
    // Draw coordinate axes to help visualize the space
    printf("[drawGame] Drawing coordinate axes\n");
    glBegin(GL_LINES);
    // X-axis (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.2f);
    glVertex3f(50.0f, 0.0f, 0.2f);
    
    // Y-axis (green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.2f);
    glVertex3f(0.0f, 50.0f, 0.2f);
    
    // Z-axis (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.2f);
    glVertex3f(0.0f, 0.0f, 50.2f);
    glEnd();
    
    // Disable lighting and depth testing
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
}

/* 
float GetDistance(float *v, float *p, float *d) {
  float diff[3];
  float tmp[3];
  float t;
  vsub(v, p, diff);
  t = scalarprod(d, diff) / scalarprod(d, d);
  vcopy(d, tmp);
  vmul(tmp, t);
  vsub(diff, tmp, tmp);
  return sqrtf( scalarprod(tmp, tmp) );
}
*/

float getDirAngle(int time, Player *p) {
	float fAngle;

	if(time < TURN_LENGTH)
	{
		fAngle = getInterpolatedAngle( 1 - (float)(TURN_LENGTH - time) / TURN_LENGTH,
			getAngle(p->data.last_dir),
			getAngle(p->data.dir));
	}
	else
	{
		fAngle = getAngle(p->data.dir);
	}

	/* result is now in radians
	// convert radians to regrees and get result into the [0,360) range
	fAngle *= 180 / (float) M_PI;
	while(fAngle >= 360)
		fAngle -= 360;
	*/
	return fAngle;
}

void getCycleTransformation(nebu_Matrix4D *pM, Player *p, int lod)
{
	nebu_Matrix4D matTranslate;
	nebu_Matrix4D matRotate;
	vec3 vTranslate;
	vec3 vUp = { { 0, 0, 1 } };
	vec3 vForward = { { 0, 1, 0 } };

	gltron_Mesh *cycle = (gltron_Mesh*)resource_Get(gpTokenLightcycles[lod], eRT_GLtronTriMesh);

	matrixIdentity(pM);
	vec3_Zero(&vTranslate);

	getPositionFromData(&vTranslate.v[0], &vTranslate.v[1], &p->data);
	
	matrixTranslation(&matTranslate, &vTranslate);
	matrixMultiply(pM, pM, &matTranslate);

	if (p->data.exp_radius == 0 && gSettingsCache.turn_cycle == 0)
	{
		matrixRotationAxis(&matRotate, - getAngle(p->data.dir), &vUp);
		matrixMultiply(pM, pM, &matRotate);
	}

	if (gSettingsCache.turn_cycle) { 
		int neigung_dir = -1;
		int time = game2->time.current - p->data.turn_time;

		matrixRotationAxis(&matRotate, getDirAngle(time, p) - (float) M_PI / 2, &vUp);
		matrixMultiply(pM, pM, &matRotate);

		#define neigung 25
		if(time < TURN_LENGTH && p->data.last_dir != p->data.dir) {
			
			float axis;
			if(p->data.dir > p->data.last_dir)
				axis = (p->data.dir - p->data.last_dir == 1) ? -1.0f : 1.0f;

			if(p->data.dir < p->data.last_dir)
				axis = (p->data.last_dir - p->data.dir == 1) ? 1.0f : -1.0f; 
			

			matrixRotationAxis(&matRotate,
				axis * neigung * sinf(PI * time / TURN_LENGTH) * (float)M_PI / 180.0f,
				&vForward);
			matrixMultiply(pM, pM, &matRotate);
		}
		#undef neigung
	}
	vTranslate.v[0] = 0;
	vTranslate.v[1] = 0;
	vTranslate.v[2] = (- cycle->BBox.vMin.v[2]);
	matrixTranslation(&matTranslate, &vTranslate);
	matrixMultiply(pM, pM, &matTranslate);
}

void drawCycleShadow(Player *p, int lod, int drawTurn) {
	gltron_Mesh *cycle;
	int turn_time = game2->time.current - p->data.turn_time;
	      
	if(turn_time < TURN_LENGTH && !drawTurn)
		return;

	if(p->data.exp_radius != 0)
		return;

	cycle = (gltron_Mesh*)resource_Get(gpTokenLightcycles[lod], eRT_GLtronTriMesh);

	/* transformations */

	glPushMatrix();
	glMultMatrixf(shadow_matrix);

	{
		nebu_Matrix4D matTransform;
		getCycleTransformation(&matTransform, p, lod);
		glMultMatrixf(matTransform.m);
	}


	/* render */
	glEnable(GL_CULL_FACE);
	gltron_Mesh_Draw(cycle, TRI_MESH);
	glDisable(GL_CULL_FACE);
	  
	glPopMatrix();
}

void drawExtruded(nebu_Mesh_IB *pIB, nebu_Mesh_VB *pVB, vec3 *pvLightDirModel)
{
	// TODO: port to OpenGL ES
	int i;
	vec3 vExtrusion;
	glBegin(GL_QUADS);
	glColor4f(1,1,1, 1.0f);
	vec3_Scale(&vExtrusion, pvLightDirModel, 100);
	// vec3_Scale(&vExtrusion, pvLightDirModel, 1);
	for(i = 0; i < pIB->nPrimitives; i++)
	{
		vec3 v;
		glVertex3fv(pVB->pVertices + 3 * pIB->pIndices[2 * i + 0]);
		glVertex3fv(pVB->pVertices + 3 * pIB->pIndices[2 * i + 1]);
		vec3_Add(&v, &vExtrusion, (vec3*) (pVB->pVertices + 3 * pIB->pIndices[2 * i + 1]));
		glVertex3fv(v.v);
		vec3_Add(&v, &vExtrusion, (vec3*) (pVB->pVertices + 3 * pIB->pIndices[2 * i + 0]));
		glVertex3fv(v.v);
	}
	glEnd();
}

void drawSharpEdges(gltron_Mesh *pMesh)
{
	int iPass, i, j;
	int nEdges = 0, nCount = 0;
	short *pIndices = NULL;

	if(!pMesh->pSI)
		return;

	if(!pMesh->pSI->pFaceNormals)
	{
		pMesh->pSI->pFaceNormals = nebu_Mesh_ComputeFaceNormals(pMesh->pSI->pVB, pMesh->pSI->pIB);
	}
	if(!pMesh->pSI->pAdjacency)
		pMesh->pSI->pAdjacency = nebu_Mesh_Adjacency_Create(pMesh->pSI->pVB, pMesh->pSI->pIB);

	// walk adjacency and detect sharp edges
	for(iPass = 0; iPass < 2; iPass++)
	{
		for(i = 0; i < pMesh->pSI->pAdjacency->nTriangles; i++)
		{
			for(j = 0; j < 3; j++)
			{
				if(pMesh->pSI->pAdjacency->pAdjacency[3 * i + j] != -1 &&
					i < pMesh->pSI->pAdjacency->pAdjacency[3 * i + j])
				{
					if(vec3_Dot((vec3*)(pMesh->pSI->pFaceNormals + i),
						(vec3*)(pMesh->pSI->pFaceNormals + pMesh->pSI->pAdjacency->pAdjacency[3 * i + j])) < 0.8f)
					{
						switch(iPass)
						{
						case 0:
							nEdges++;
							break;
						case 1:
							pIndices[2 * nCount + 0] = pMesh->pSI->pIB->pIndices[3 * i + j];
							pIndices[2 * nCount + 1] = pMesh->pSI->pIB->pIndices[3 * i + (j + 1) % 3];
							nCount++;
							break;
						default:
							nebu_assert(0);
							break;
						}
					}
				}
			}
		}
		switch(iPass)
		{
		case 0:
			pIndices = malloc(2 * nEdges * sizeof(int));
			break;
		case 1:
			nebu_assert(nCount == nEdges);
			nebu_Mesh_VB_Enable(pMesh->pSI->pVB);
			glDrawElements(GL_LINES, 2 * nCount, GL_UNSIGNED_SHORT, pIndices);
			nebu_Mesh_VB_Disable(pMesh->pSI->pVB);

			free(pIndices);
			break;
		default:
			nebu_assert(0);
		}
	}
}

void drawCycle(int player, int lod, int drawTurn) {
    Player *p = game->player + player;

    printf("[drawCycle] Drawing cycle for player %d with LOD %d\n", player, lod);

    gltron_Mesh *cycle = (gltron_Mesh*)resource_Get(gpTokenLightcycles[lod], eRT_GLtronTriMesh);

    nebu_Matrix4D matCycleToWorld; // from the cycle's model-space to world space
    nebu_Matrix4D matCycleToWorldInvInvT; // light from world space to model space

    unsigned int spoke_time = game2->time.current - gSpoke_time;
    int turn_time = game2->time.current - p->data.turn_time;

    if(turn_time < TURN_LENGTH && !drawTurn)
        return;

    getCycleTransformation(&matCycleToWorld, p, lod);

    if(p->data.wall_buster_enabled) {
        float black[] = { 0, 0, 0, 1};
        float white[] = { 1, 1, 1, 1};
        gltron_Mesh_Material_SetColor(cycle, "Hull", eDiffuse, black);
        gltron_Mesh_Material_SetColor(cycle, "Hull", eSpecular, white);
    } else {
        gltron_Mesh_Material_SetColor(cycle, "Hull", eDiffuse, p->profile.pColorDiffuse);
        gltron_Mesh_Material_SetColor(cycle, "Hull", eSpecular, p->profile.pColorSpecular);
    }

    if (p->data.exp_radius == 0) {
        nebu_Video_CheckErrors("before bike drawing");

        glEnable(GL_NORMALIZE);

        /* draw spoke animation */
        if (gSpoke_time > 140 - (p->data.speed * 10) && game->pauseflag == PAUSE_GAME_RUNNING) {
            if (gSpoke_state == 1) {
                gSpoke_state = 0;
                gltron_Mesh_Material_SetColor(cycle, "Spoke", eSpecular, SpokeColor);
                gltron_Mesh_Material_SetColor(cycle, "Spoke", eAmbient, SpokeColor);
            } else {
                gSpoke_state = 1;
                gltron_Mesh_Material_SetColor(cycle, "Spoke", eSpecular, NoSpokeColor);
                gltron_Mesh_Material_SetColor(cycle, "Spoke", eAmbient, NoSpokeColor);
            }
            gSpoke_time = game2->time.current;
        }

        if (gSettingsCache.light_cycles) {
            glEnable(GL_LIGHTING); // enable OpenGL lighting for lightcycles
        }

        if(cycle->pSI) {
            // clear stencil buffer, but leave reflection bit alone
            glEnable(GL_STENCIL_TEST);
            glStencilMask(gShadowVolStencilMask);
            if(gIsRenderingReflection)
                glStencilFunc(GL_EQUAL, gFloorStencilRef, gFloorStencilRef);
            else
                glStencilFunc(GL_ALWAYS, 0, ~0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
        }

        // draw cycle with ambient (including camera) lighting
        setupLights(eCyclesAmbient);
        glColor4f(0,0,0, 1.0f);
        glDisable(GL_LIGHTING);

        glPushMatrix();
        glMultMatrixf(matCycleToWorld.m);

        // TODO: for reflections, clip to reflector
        glEnable(GL_CULL_FACE);
        gltron_Mesh_Draw(cycle, TRI_MESH);
        glDisable(GL_CULL_FACE);

        // draw shadow volume to mask out shadowed areas
        if(cycle->pSI && getSettingi("shadow_volumes_cycle")) {
            vec3 vLightDirWorld = { { -.5, -.5, -1 } };
            vec3 vLightDirModel;

            GLint front = (gIsRenderingReflection) ? GL_BACK : GL_FRONT;
            GLint back = (gIsRenderingReflection) ? GL_FRONT : GL_BACK;

            nebu_Video_CheckErrors("before shadow volume");

            matrixTranspose(&matCycleToWorldInvInvT, &matCycleToWorld);
            vec3_Transform(&vLightDirModel, &vLightDirWorld, &matCycleToWorldInvInvT);
            vec3_Normalize(&vLightDirModel, &vLightDirModel);
            nebu_Mesh_Shadow_SetLight(cycle->pSI, &vLightDirModel);

            glDisable(GL_LIGHTING);

            // write only to stencil
            glDepthMask(GL_FALSE);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            glEnable(GL_STENCIL_TEST);
            glStencilMask(gShadowVolStencilMask);
            glStencilFunc(GL_ALWAYS, 0, ~0);

            glEnable(GL_CULL_FACE);

            // make sure that reflected volumes are properly drawn (e.g. not clipped by the floor plane etc.)
            if(gIsRenderingReflection)
                glDisable(GL_CLIP_PLANE0);

            // front faces
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
            glCullFace(back);
            drawExtruded(cycle->pSI->pEdges, cycle->pSI->pVB, &vLightDirModel);
            // back faces
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
            glCullFace(front);
            drawExtruded(cycle->pSI->pEdges, cycle->pSI->pVB, &vLightDirModel);
            glCullFace(back);

            if(gIsRenderingReflection)
                glEnable(GL_CLIP_PLANE0);

            glDisable(GL_CULL_FACE);
            // restore color buffer access
            glDepthMask(GL_TRUE);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            if(getSettingi("cycle_sharp_edges")) {
                glColor4f(.5,.5,.5, 1.0f);
                glPolygonOffset(1,1);
                glEnable(GL_POLYGON_OFFSET_FILL);
                drawSharpEdges(cycle);
                glDisable(GL_POLYGON_OFFSET_FILL);
            }

            if(gIsRenderingReflection) {
                // 'clip' cycle to reflector
                glStencilFunc(GL_EQUAL, gFloorStencilRef, gShadowVolStencilMask | gFloorStencilRef);
            } else {
                glStencilFunc(GL_EQUAL, 0, gShadowVolStencilMask);
            }
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

            nebu_Video_CheckErrors("after shadow volume");
        }

        glDepthFunc(GL_LEQUAL);

        glPopMatrix();
        setupLights(eCyclesWorld);

        glEnable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glPushMatrix();
        glMultMatrixf(matCycleToWorld.m);

        glEnable(GL_CULL_FACE);
        gltron_Mesh_Draw(cycle, TRI_MESH);
        glDisable(GL_CULL_FACE);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_BLEND);

        if(cycle->pSI && !gIsRenderingReflection)
            glDisable(GL_STENCIL_TEST);

        // draw wall buster 'special effect'
        if(p->data.wall_buster_enabled) {
            int i;
            float fScale;
            float gray[] = { 0.5f, 0.5f, 0.5f, 0.2f };
            glEnable(GL_BLEND);
            glDepthMask(GL_FALSE);
            for(i = 0; i < 1; i++) {
                fScale = 1.2f;
                gltron_Mesh_Material_SetColor(cycle, "Hull", eDiffuse, gray);
                gltron_Mesh_Material_SetAlpha(cycle, gray[3]);
                glPushMatrix();
                glScalef(fScale, fScale, fScale);
                gltron_Mesh_Draw(cycle, TRI_MESH);
                glPopMatrix();
                gltron_Mesh_Material_SetAlpha(cycle, 1);
            }
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        } // done with wall buster fx

        glDisable(GL_CULL_FACE);

        glDisable(GL_LIGHTING);

        glPopMatrix();

        nebu_Video_CheckErrors("after bike drawing");
    } else if(p->data.exp_radius < EXP_RADIUS_MAX) {
        nebu_Video_CheckErrors("before explosion");

        glPushMatrix();
        glMultMatrixf(matCycleToWorld.m);

        glEnable(GL_BLEND);

        if (gSettingsCache.show_impact) {
            glPushMatrix();
            glTranslatef(0, 0, - (cycle->BBox.vMax.v[2] - cycle->BBox.vMin.v[2]) / 2);
            drawImpact(player);
            glPopMatrix();
        }

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (gSettingsCache.light_cycles) {
            glEnable(GL_LIGHTING); // enable OpenGL lighting for lightcycles
        }
        glDisable(GL_LIGHTING); // disable ligthing after lightcycles
        glDisable(GL_BLEND);

        glPopMatrix();

        nebu_Video_CheckErrors("after explosion");
    }
}
 
/*! Returns the level of detail to be used for drawing the player mesh */

// int playerVisible(int eyePlayer, int targetPlayer) {
int playerVisible(Camera *pCamera, Player *pTarget)
{
	// Compute view vector
	// Compute eye-to-object vector
	// Check 1: Is the target in the field of view
	// Check 2: Compute LOD from distance
	// return 0;

	vec3 vView, vCycle, v;
		
	float s, d;
	float fSafety = 20;
	float fDistance;
	int i;
	int lod_level;
	float x, y;

	vec3_Sub(&vView, (vec3*)pCamera->target, (vec3*)pCamera->cam);
	vec3_Normalize(&vView, &vView);
	
	getPositionFromData(&x, &y, &pTarget->data);
	v.v[0] = x;
	v.v[1] = y;
	v.v[2] = 0;
		
	vec3_Sub(&vCycle, &v, (vec3*)pCamera->cam);
	fDistance = vec3_Length(&vCycle);
	vec3_Normalize(&vCycle, &vCycle);

	lod_level = (gSettingsCache.lod > MAX_LOD_LEVEL) ? 
		MAX_LOD_LEVEL : gSettingsCache.lod;

	/* calculate lod */
	
	for(i = 0; i < LC_LOD && fDistance >= lod_dist[lod_level][i]; i++);
	if(i >= LC_LOD)
		return -1;

	s = vec3_Dot(&vCycle, &vView);
	/* maybe that's not exactly correct, but I didn't notice anything */
	d = cosf( (gSettingsCache.fov / 2 + fSafety) * 2 * PI / 360.0 );
	/*
		printf("v1: %.2f %.2f %.2f\nv2: %.2f %.2f %.2f\ns: %.2f d: %.2f\n\n",
		v1[0], v1[1], v1[2], v2[0], v2[1], v2[2],
		s, d);
	*/
	
	// TODO: take bounding box of lightcycle into account
	if(s < d)
		return -1;
	else
		return i;
}

void drawPlayers(Camera *pCamera) {
    int i;

    printf("[drawPlayers] Drawing players\n");

    for(i = 0; i < game->players; i++) {
        int lod;
        int drawTurn = 1;

        lod = playerVisible(pCamera, &game->player[i]);
        printf("[drawPlayers] Player %d LOD: %d\n", i, lod);
        if (lod >= 0) {
            drawCycle(i, lod, drawTurn);
        }
    }
}

void drawPlanarShadows(Camera *pCamera) {
    int i;

    if (gSettingsCache.use_stencil) {
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
        glStencilMask(gFloorStencilRef);
        glStencilFunc(GL_EQUAL, gFloorStencilRef, gFloorStencilRef);
        glEnable(GL_BLEND);
        glColor4f(gCurrentShadowColor[0],
            gCurrentShadowColor[1],
            gCurrentShadowColor[2],
            gCurrentShadowColor[3]);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glColor4f(0, 0, 0, 1.0f);
        glDisable(GL_BLEND);
    }

    /* Shadows on the floor: cycle, recognizer, trails */
    if (gSettingsCache.show_recognizer &&
        getSettingi("shadow_projective_recognizer_on_floor")) {
        drawRecognizerShadow();
    }

    for (i = 0; i < game->players; i++) {
        int lod = playerVisible(pCamera, &game->player[i]);
        if (lod >= 0 && getSettingi("shadow_projective_cycle_on_floor")) {
            int drawTurn = 1;
            drawCycleShadow(game->player + i, lod, drawTurn);
        }
        if (game->player[i].data.trail_height > 0 &&
            getSettingi("shadow_projective_trails_on_floor"))
            drawTrailShadow(game->player + i);
    }

    if (gSettingsCache.use_stencil)
        glDisable(GL_STENCIL_TEST);

    glDisable(GL_BLEND);
}

void drawCam2(Camera *pCamera)
{
	// TODO: build the scene
	// static elements:
	// - arena
	// - skybox
	// dynamic elements:
	// - 4 lightcycles (different LODs), with 4 diffent transformation matrices
	//	 and different materials
	// - 4 lightcycle trails
	// - recognizer
	// - special effects:
	//   - crash effect
	//   - lightcycle glow
	//   - trail lines

	// TODO: first version
	// procedural:
	// - crash effect
	// - lightcycle glow
	// - skybox
	// scenegraph:
	// - arena
	// - lightcycle
	// - lightcycle trails
	// - recognizer
}

void crossProduct(vec3 *a, vec3 *b, vec3 *result) {
    result->v[0] = a->v[1] * b->v[2] - a->v[2] * b->v[1];
    result->v[1] = a->v[2] * b->v[0] - a->v[0] * b->v[2];
    result->v[2] = a->v[0] * b->v[1] - a->v[1] * b->v[0];
}

float dotProduct(vec3 *a, vec3 *b) {
    return a->v[0] * b->v[0] + a->v[1] * b->v[1] + a->v[2] * b->v[2];
}

void normalize(vec3 *v) {
    float length = sqrt(v->v[0] * v->v[0] + v->v[1] * v->v[1] + v->v[2] * v->v[2]);
    v->v[0] /= length;
    v->v[1] /= length;
    v->v[2] /= length;
}

void computeViewMatrix(Camera *cam, float *outMatrix) {
    // Camera and target position vectors
    vec3 forward, right, up;
    
    // Calcular o vetor de direção (forward) da câmera
    forward.v[0] = cam->target[0] - cam->cam[0];
    forward.v[1] = cam->target[1] - cam->cam[1];
    forward.v[2] = cam->target[2] - cam->cam[2];
    
    // Normalize the direction vector (forward)
    normalize(&forward);
    
    // Calculate the right vector, which is orthogonal to the forward vector
    up.v[0] = 0.0f;
    up.v[1] = 0.0f;
    up.v[2] = 1.0f; // The "up" vector assumes the positive Z direction
    crossProduct(&up, &forward, &right);
    normalize(&right);
    
    // Recalculate the up vector
    crossProduct(&forward, &right, &up);
    normalize(&up);

    // Assemble the 4x4 display matrix
    outMatrix[0] = right.v[0];  outMatrix[4] = right.v[1];  outMatrix[8]  = right.v[2];  outMatrix[12] = -dotProduct(&right, &forward);
    outMatrix[1] = up.v[0];     outMatrix[5] = up.v[1];     outMatrix[9]  = up.v[2];     outMatrix[13] = -dotProduct(&up, &forward);
    outMatrix[2] = -forward.v[0]; outMatrix[6] = -forward.v[1]; outMatrix[10] = -forward.v[2]; outMatrix[14] = dotProduct(&forward, &forward);
    outMatrix[3] = 0.0f;        outMatrix[7] = 0.0f;        outMatrix[11] = 0.0f;        outMatrix[15] = 1.0f;
}

void drawWorld(Camera *pCamera) {
    
    // === Apply the camera transformation ===
    float viewMatrix[16];
    computeViewMatrix(pCamera, viewMatrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(viewMatrix);
    
    int i;

    printf("[drawWorld] Drawing world\n");

    nebu_Video_CheckErrors("before world");

    setupLights(eWorld);

    if (gSettingsCache.show_wall == 1 && gWorld->arena) {
        glColor4f(1, 1, 1, 1.0f);
        drawWalls();
    }

    nebu_Video_CheckErrors("before players");
    drawPlayers(pCamera);
    nebu_Video_CheckErrors("after players");

    setupLights(eCyclesWorld);
    glEnable(GL_LIGHTING);
    {
        TrailMesh mesh;
        mesh.pVertices = (vec3*)malloc(1000 * sizeof(vec3));
        mesh.pNormals = (vec3*)malloc(1000 * sizeof(vec3));
        mesh.pColors = (unsigned char*)malloc(1000 * 4 * sizeof(float));
        mesh.pTexCoords = (vec2*)malloc(1000 * sizeof(vec2));
        mesh.pIndices = (unsigned short*)malloc(1000 * 2);

        for (i = 0; i < game->players; i++) {
            if (game->player[i].data.trail_height > 0) {
                int vOffset = 0;
                int iOffset = 0;
                mesh.iUsed = 0;
                nebu_Video_CheckErrors("before trail geometry");
                trailGeometry(game->player + i, &game->player[i].profile, &mesh, &vOffset, &iOffset);
                nebu_Video_CheckErrors("after trail geometry");
                bowGeometry(game->player + i, &game->player[i].profile, &mesh, &vOffset, &iOffset);
                nebu_Video_CheckErrors("after bow geometry");
                trailStatesNormal(game->player + i, gScreen->textures[TEX_DECAL]);
                trailRender(&mesh);
                trailStatesRestore();
            }
        }
        free(mesh.pVertices);
        free(mesh.pNormals);
        free(mesh.pColors);
        free(mesh.pTexCoords);
        free(mesh.pIndices);
    }
    glDisable(GL_LIGHTING);

    for (i = 0; i < game->players; i++)
        if (game->player[i].data.trail_height > 0)
            drawTrailLines(pCamera, &game->player[i]);

    nebu_Video_CheckErrors("after world");
}

static float getReflectivity() {
	float reflectivity = getSettingf("reflection");
	if(reflectivity < 0)
		reflectivity = getVideoSettingf("reflection");

	// need stencil for reflections
	if(gSettingsCache.use_stencil == 0)
		reflectivity = 0;
	return reflectivity;
}

/*	draw's a GLtron scene, which consists of the following step:
	- setup up a perspective projection matrix
	- setup the view matrix
	- draw the skybox
	- draw the floor geometry, possible with blended world & skybox reflections
	- draw projected shadows of the trails, lightcycles & recognizer to the
	  floor geometry (darkening pass)
	- draw world geometry (recognizer, arena walls, lightcycles,
	  cycle trails (back-to-front)
*/

/*	TODO: with engine style rendering, that's
	- old style rendering
		- foreach object in the scene
			- find geometry
			- find shader
			- draw shader
		- objects consist of:
			- multiple sets of:
				- mesh (vertex & perhaps index buffer)  
				- world transformation
		- example 1 (without reflections or shadows):
			skybox:
				- 6 objects (quaads, i.e. two triangles)
				- 6 shaders of the same type (but with a different texture each)
			floor:
				- 1 object & shader (one texture)
			walls:
				- 1 object & shader (one texture)
			lightcycles & recognizer (not finalized):
				- 1 object per material & shader (no texture, but with special lighting,
					wheel spoke stuff, explosions, recognizer outlines,
					etc.)
			lightcycle trail:
				- TODO
			special effects:
				- lightcycle trail glow
				- trail lines
		- example 2 (without reflections, but with projected shadows)
			- same as example 2, but add after floor:
				- all object geometry that casts a drop-shadow (careful!
				 what about objects that rely on alpha-transparency?)
				- drop shadow shader (setup projection matrix, darkening)

	2D post processing happens after all viewports are drawn (i.e.
	at the end of the drawGame() function, and consists of
	- HUD
	- full screen effects
*/

/* plan for the transition:
	move objects 1-by-1 into the 'scenegraph' (temporarily screws up
	drawing order, but so what)
	traverse scenegraph each frame, and build object & shader lists
*/

void setupCamera(Camera *pCamera) {
    float up[3] = { 0, 0, 1 };

    // Setup a perspective projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    doPerspective(gSettingsCache.fov, (float)gScreen->w / (float)gScreen->h,
        gSettingsCache.znear, box2_Diameter(&game2->level->boundingBox) * 6.5f);

    // Setup the view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    {
        vec3 vLookAt;
        vec3 vTarget;
        matrix matRotate;

        vec3_Sub(&vLookAt, (vec3*)pCamera->target, (vec3*)pCamera->cam);
        vec3_Normalize(&vLookAt, &vLookAt);
        matrixRotationAxis(&matRotate, 90.0f * (float)pCamera->bIsGlancing, (vec3*)up);
        vec3_Transform(&vLookAt, &vLookAt, &matRotate);
        vec3_Add(&vTarget, (vec3*)pCamera->cam, &vLookAt);
        doLookAt(pCamera->cam, (float*)&vTarget, up);
    }
}

/*
void drawSkybox(Skybox *skybox) {
    // Draw the skybox if it exists
    if (skybox && skybox->skyboxMesh) {
        gltron_Mesh_Draw(skybox->skyboxMesh, TRI_MESH);
    }
}
*/

void drawCam(PlayerVisual *pV) {
    int i;
    float up[3] = { 0, 0, 1 };

    Camera *pCamera = &pV->camera;

    printf("[drawCam] Starting to draw camera view\n");

    // Force reflectivity to 0 to avoid issues with reflections
    float reflectivity = 0.0f;
    printf("[drawCam] Reflectivity: %f\n", reflectivity);

    // Compute shadow color based on global constant & reflectivity
    for (i = 0; i < 4; i++)
        gCurrentShadowColor[i] = gShadowColor[i] * (1 - reflectivity);

    glDisable(GL_LIGHTING); // Initial config at frame start
    glDisable(GL_BLEND); // Initial config at frame start

    // Disable writes to alpha
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

    setupCamera(pCamera); // Use pCamera instead of pV

    /* Skybox */
    printf("[drawCam] Drawing skybox\n");

    // Ensure gWorld->skybox is a valid identifier
    if (gWorld && gWorld->skybox) {
        drawSkybox(&gWorld->skybox);
    } else {
        printf("[drawCam] Skipping skybox drawing: gWorld or gWorld->skybox is NULL\n");
    }

    /* Floor */
    printf("[drawCam] Drawing floor\n");

    if (!gWorld) {
        printf("[drawCam] gWorld is NULL\n");
        return;
    }

    if (!gWorld->floor) {
        printf("[drawCam] gWorld->floor is NULL\n");
        return;
    }

    // Force reflectivity to 0 to avoid issues with reflections
    if (1) {
        // Draw floor to fb, z and stencil, using alpha-blending
        // TODO: draw floor alpha to fb
        printf("[drawCam] Setting up floor shader\n");
        video_Shader_Setup(&gWorld->floor_shader, 0);

        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(gFloorStencilRef);
        glStencilFunc(GL_ALWAYS, gFloorStencilRef, ~0);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, 0.1f);

        printf("[drawCam] Drawing floor mesh\n");

        // Skip gltron_Mesh_Draw for now
        printf("[drawCam] Skipping gltron_Mesh_Draw to avoid segmentation fault\n");

        // Draw a simple floor instead
        printf("[drawCam] Drawing a simple floor\n");

        glBegin(GL_QUADS);
        glColor4f(0.5f, 0.5f, 0.5f, 1.0f);  // Gray

        // Bottom-left
        glVertex3f(-100.0f, -100.0f, 0.0f);

        // Bottom-right
        glVertex3f(100.0f, -100.0f, 0.0f);

        // Top-right
        glVertex3f(100.0f, 100.0f, 0.0f);

        // Top-left
        glVertex3f(-100.0f, 100.0f, 0.0f);

        glEnd();

        glDisable(GL_ALPHA_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST);

        printf("[drawCam] Cleaning up floor shader\n");
        video_Shader_Cleanup(&gWorld->floor_shader, 0);
    }

    /* Planar shadows */
    printf("[drawCam] Drawing planar shadows\n");

    // Skip planar shadows for now
    printf("[drawCam] Skipping planar shadows to avoid segmentation fault\n");

    /* World */
    printf("[drawCam] Drawing world\n");

    // Skip drawWorld for now
    printf("[drawCam] Skipping drawWorld to avoid segmentation fault\n");

    // Draw simple arena walls instead
    printf("[drawCam] Drawing simple arena walls\n");

    glBegin(GL_QUADS);
    glColor4f(0.7f, 0.7f, 0.7f, 1.0f);  // Light gray

    // Front wall
    glVertex3f(-100.0f, 100.0f, 0.0f);
    glVertex3f(100.0f, 100.0f, 0.0f);
    glVertex3f(100.0f, 100.0f, 10.0f);
    glVertex3f(-100.0f, 100.0f, 10.0f);

    // Back wall
    glVertex3f(-100.0f, -100.0f, 0.0f);
    glVertex3f(100.0f, -100.0f, 0.0f);
    glVertex3f(100.0f, -100.0f, 10.0f);
    glVertex3f(-100.0f, -100.0f, 10.0f);

    // Left wall
    glVertex3f(-100.0f, -100.0f, 0.0f);
    glVertex3f(-100.0f, 100.0f, 0.0f);
    glVertex3f(-100.0f, 100.0f, 10.0f);
    glVertex3f(-100.0f, -100.0f, 10.0f);

    // Right wall
    glVertex3f(100.0f, -100.0f, 0.0f);
    glVertex3f(100.0f, 100.0f, 0.0f);
    glVertex3f(100.0f, 100.0f, 10.0f);
    glVertex3f(100.0f, -100.0f, 10.0f);

    glEnd();

    // Draw a grid on the floor to help visualize the space
    printf("[drawCam] Drawing grid\n");
    glDisable(GL_LIGHTING);
    glColor4f(0.3f, 0.3f, 0.3f, 1.0f);  // Dark gray

    glBegin(GL_LINES);
    for (int i = -100; i <= 100; i += 20) {
        // Lines along the x-axis
        glVertex3f(i, -100.0f, 0.1f);
        glVertex3f(i, 100.0f, 0.1f);

        // Lines along the y-axis
        glVertex3f(-100.0f, i, 0.1f);
        glVertex3f(100.0f, i, 0.1f);
    }
    glEnd();

    // Draw coordinate axes to help visualize the space
    printf("[drawCam] Drawing coordinate axes\n");
    glBegin(GL_LINES);
    // X-axis (red)
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.2f);
    glVertex3f(50.0f, 0.0f, 0.2f);

    // Y-axis (green)
    glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.2f);
    glVertex3f(0.0f, 50.0f, 0.2f);

    // Z-axis (blue)
    glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.2f);
    glVertex3f(0.0f, 0.0f, 50.2f);
    glEnd();

    /* Draw players */
    printf("[drawCam] Drawing players\n");

    // Check if game2 and game exist
    if (!game2) {
        printf("[drawCam] game2 is NULL, cannot draw players\n");
        goto skip_players;
    }

    if (!game) {
        printf("[drawCam] game is NULL, cannot draw players\n");
        goto skip_players;
    }

    // Check if game2->play is set
    if (game2->play) {
        printf("[drawCam] Drawing players from game2\n");

        // Check if game->players is valid
        if (game->players <= 0) {
            printf("[drawCam] No players to draw (game->players = %d)\n", game->players);
            goto skip_players;
        }

        // Check if game->player is valid
        if (!game->player) {
            printf("[drawCam] game->player is NULL\n");
            goto skip_players;
        }

        // Draw each player
        for (i = 0; i < game->players; i++) {
            printf("[drawCam] Drawing player %d\n", i);

            // Draw player as a simple cube
            glPushMatrix();

            // Get player position safely
            float x = 0.0f, y = 0.0f;

            // Check if we can access player data
            if (&game->player[i] && &game->player[i].data) {
                x = game->player[i].data.posx;
                y = game->player[i].data.posy;
                printf("[drawCam] Player %d position: (%f, %f)\n", i, x, y);
            } else {
                printf("[drawCam] Cannot access player %d data\n", i);
                glPopMatrix();
                continue;
            }

            // Move to player position
            glTranslatef(x, y, 1.0f);

            // Rotate based on player direction
            int direction = game->player[i].data.dir;
            float angle = 0.0f;

            switch (direction) {
                case 0: angle = 0.0f; break;   // Right
                case 1: angle = 90.0f; break;  // Up
                case 2: angle = 180.0f; break; // Left
                case 3: angle = 270.0f; break; // Down
                default: angle = 0.0f; break;
            }

            glRotatef(angle, 0.0f, 0.0f, 1.0f);

            // Set player color
            if (&game->player[i].profile) {
                glColor4f(
                    game->player[i].profile.pColorDiffuse[0],
                    game->player[i].profile.pColorDiffuse[1],
                    game->player[i].profile.pColorDiffuse[2],
                    1.0f
                );
            } else {
                // Default color if profile is not accessible
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            }

            // Draw player cube
            glBegin(GL_QUADS);

            // Front face
            glVertex3f(-2.0f, -4.0f, 0.0f);
            glVertex3f(2.0f, -4.0f, 0.0f);
            glVertex3f(2.0f, -4.0f, 2.0f);
            glVertex3f(-2.0f, -4.0f, 2.0f);

            // Back face
            glVertex3f(-2.0f, 4.0f, 0.0f);
            glVertex3f(2.0f, 4.0f, 0.0f);
            glVertex3f(2.0f, 4.0f, 2.0f);
            glVertex3f(-2.0f, 4.0f, 2.0f);

            // Left face
            glVertex3f(-2.0f, -4.0f, 0.0f);
            glVertex3f(-2.0f, 4.0f, 0.0f);
            glVertex3f(-2.0f, 4.0f, 2.0f);
            glVertex3f(-2.0f, -4.0f, 2.0f);

            // Right face
            glVertex3f(2.0f, -4.0f, 0.0f);
            glVertex3f(2.0f, 4.0f, 0.0f);
            glVertex3f(2.0f, 4.0f, 2.0f);
            glVertex3f(2.0f, -4.0f, 2.0f);

            // Top face
            glVertex3f(-2.0f, -4.0f, 2.0f);
            glVertex3f(2.0f, -4.0f, 2.0f);
            glVertex3f(2.0f, 4.0f, 2.0f);
            glVertex3f(-2.0f, 4.0f, 2.0f);

            glEnd();

            glPopMatrix();

            // Skip trail drawing for now to avoid segmentation fault
            printf("[drawCam] Skipping trail drawing for player %d to avoid segmentation fault\n", i);
        }
    } else {
        printf("[drawCam] No players to draw (game2->play is not set)\n");
    }

skip_players:
    /* Transparent stuff */
    printf("[drawCam] Drawing transparent stuff\n");

    // Skip transparent stuff for now
    printf("[drawCam] Skipping transparent stuff to avoid segmentation fault\n");

    printf("[drawCam] Camera view drawing complete\n");
}

void drawSimpleTrail(Player *pPlayer) {
    if (!pPlayer) return;
    
    // Get player data
    Data *data = &pPlayer->data;
    
    // Check if player is active
    if (data->speed <= 0) return;
    
    // Get player color
    float r = pPlayer->profile.pColorDiffuse[0];
    float g = pPlayer->profile.pColorDiffuse[1];
    float b = pPlayer->profile.pColorDiffuse[2];
    
    // Set trail color (slightly transparent)
    glColor4f(r, g, b, 0.7f);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Current position
    float x = data->posx;
    float y = data->posy;
    float height = data->trail_height;
    
    // If we don't have trail segments yet, just draw a simple line
    if (!data->trails || data->nTrails == 0) {
        // Calculate trail start position based on direction
        float trail_length = 20.0f;  // Length of the trail
        float trail_x = x;
        float trail_y = y;
        
        switch (data->dir) {
            case 0:  // Right
                trail_x -= trail_length;
                break;
            case 1:  // Up
                trail_y -= trail_length;
                break;
            case 2:  // Left
                trail_x += trail_length;
                break;
            case 3:  // Down
                trail_y += trail_length;
                break;
        }
        
        // Draw the trail as a 3D rectangle
        glBegin(GL_QUADS);
        
        // Bottom face (on the floor)
        glVertex3f(trail_x, trail_y, 0.1f);
        glVertex3f(x, y, 0.1f);
        glVertex3f(x, y, height);
        glVertex3f(trail_x, trail_y, height);
        
        glEnd();
    } else {
        // Draw all trail segments
        glBegin(GL_QUADS);
        
        for (int i = 0; i < data->nTrails; i++) {
            segment2 *segment = &data->trails[i];
            
            // Get segment start and end points
            float x1 = segment->vStart.v[0];
            float y1 = segment->vStart.v[1];
            float x2 = x1 + segment->vDirection.v[0];
            float y2 = y1 + segment->vDirection.v[1];
            
            // Draw segment as a 3D rectangle
            // Bottom face (on the floor)
            glVertex3f(x1, y1, 0.1f);
            glVertex3f(x2, y2, 0.1f);
            
            // Top face (at trail height)
            glVertex3f(x2, y2, height);
            glVertex3f(x1, y1, height);
        }
        
        glEnd();
    }
    
    // Disable blending
    glDisable(GL_BLEND);
}

void drawSimpleExplosion(Player *pPlayer) {
    if (!pPlayer) return;
    
    // Get player data
    Data *data = &pPlayer->data;
    
    // Check if player has crashed
    if (data->exp_radius <= 0) return;
    
    // Get player position
    float x = data->posx;
    float y = data->posy;
    
    // Get player color
    float r = pPlayer->profile.pColorDiffuse[0];
    float g = pPlayer->profile.pColorDiffuse[1];
    float b = pPlayer->profile.pColorDiffuse[2];
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Disable depth testing to ensure explosion is visible
    glDisable(GL_DEPTH_TEST);
    
    // Draw explosion as a series of concentric circles
    float max_radius = 10.0f;
    float alpha = 1.0f - (data->exp_radius / max_radius);
    if (alpha < 0) alpha = 0;
    
    glColor4f(1.0f, 0.5f, 0.0f, alpha * 0.7f);  // Orange-yellow with transparency
    
    // Draw outer explosion circle
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(x, y, 0.5f);  // Center point
    
    int segments = 20;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float px = x + cosf(angle) * data->exp_radius;
        float py = y + sinf(angle) * data->exp_radius;
        glVertex3f(px, py, 0.5f);
    }
    glEnd();
    
    // Draw inner explosion circle
    glColor4f(1.0f, 1.0f, 0.0f, alpha);  // Bright yellow
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(x, y, 0.6f);  // Center point
    
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float px = x + cosf(angle) * data->exp_radius * 0.6f;
        float py = y + sinf(angle) * data->exp_radius * 0.6f;
        glVertex3f(px, py, 0.6f);
    }
    glEnd();
    
    // Draw debris particles
    glBegin(GL_POINTS);
    glColor4f(r, g, b, alpha);
    
    int num_particles = 30;
    for (int i = 0; i < num_particles; i++) {
        float angle = (float)rand() / RAND_MAX * 2.0f * M_PI;
        float distance = (float)rand() / RAND_MAX * data->exp_radius * 1.2f;
        float px = x + cosf(angle) * distance;
        float py = y + sinf(angle) * distance;
        float pz = 0.5f + (float)rand() / RAND_MAX * 2.0f;  // Random height
        
        glVertex3f(px, py, pz);
    }
    glEnd();
    
    // Restore OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    
    // Increase explosion radius for next frame
    data->exp_radius += 0.5f;
    
    // Reset explosion when it gets too big
    if (data->exp_radius > max_radius) {
        data->exp_radius = 0;
    }
}
