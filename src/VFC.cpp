#include "VFC.h"

#define PLAYER_CAM_DIST 60.0f
#define CULL_DIST 100.0f
#define VFC_BUF 6

using namespace std;
using namespace glm;

mat4 SetProjectionMatrix(shared_ptr<Program> curShade, WindowManager *WM, int width, int height) {
    glfwGetFramebufferSize(WM->getHandle(), &width, &height);
    float aspect = width/(float)height;
    mat4 Projection = perspective(radians(PLAYER_CAM_DIST), aspect, 0.1f, CULL_DIST);
    glUniformMatrix4fv(curShade->getUniform("P"), 1, GL_FALSE, value_ptr(Projection));
    return Projection;
}

mat4 SetOrthoMatrix(shared_ptr<Program> curShade) {
    float wS = 2.5;
    mat4 ortho = glm::ortho(-15.0f*wS, 15.0f*wS, -15.0f*wS, 15.0f*wS, 2.1f, 100.f);
    glUniformMatrix4fv(curShade->getUniform("P"), 1, GL_FALSE, value_ptr(ortho));
    return ortho;
}

/* camera controls - this is the camera for the top down view */
mat4 SetTopView(shared_ptr<Program> curShade, vec3 camPosition, vec3 position) {
    mat4 Cam = lookAt(camPosition + vec3(0, 8, 0), camPosition, position - camPosition);
    glUniformMatrix4fv(curShade->getUniform("V"), 1, GL_FALSE, value_ptr(Cam));
    return Cam;
}

/*normal game camera */
mat4 SetView(shared_ptr<Program> curShade, vec3 camPosition, vec3 position) {
    mat4 Cam = lookAt(camPosition, position, vec3(0, 1, 0));
    glUniformMatrix4fv(curShade->getUniform("V"), 1, GL_FALSE, value_ptr(Cam));
    return Cam;
}

vec4 Left, Right, Bottom, Top, Near, Far;
vec4 planes[6];

void ExtractVFPlanes(mat4 P, mat4 V) {
    /* composite matrix */
    mat4 comp = P*V;
    float magnitude;
    vec3 n; //use to pull out normal
    float l; //length of normal for plane normalization
    
    Left.x = comp[0][3] + comp[0][0]; // see handout to fill in with values from comp
    Left.y = comp[1][3] + comp[1][0]; // see handout to fill in with values from comp
    Left.z = comp[2][3] + comp[2][0]; // see handout to fill in with values from comp
    Left.w = comp[3][3] + comp[3][0]; // see handout to fill in with values from comp
    magnitude = sqrt(pow(Left.x, 2) + pow(Left.y, 2) + pow(Left.z, 2));
    Left.x = Left.x / magnitude;
    Left.y = Left.y / magnitude;
    Left.z = Left.z / magnitude;
    Left.w = Left.w / magnitude;
    planes[0] = Left;
    //cout << "Left' " << Left.x << " " << Left.y << " " <<Left.z << " " << Left.w << endl;
    
    Right.x = comp[0][3] - comp[0][0]; // see handout to fill in with values from comp
    Right.y = comp[1][3] - comp[1][0]; // see handout to fill in with values from comp
    Right.z = comp[2][3] - comp[2][0]; // see handout to fill in with values from comp
    Right.w = comp[3][3] - comp[3][0]; // see handout to fill in with values from comp
    magnitude = sqrt(pow(Right.x, 2) + pow(Right.y, 2) + pow(Right.z, 2));
    Right.x = Right.x / magnitude;
    Right.y = Right.y / magnitude;
    Right.z = Right.z / magnitude;
    Right.w = Right.w / magnitude;
    planes[1] = Right;
    //cout << "Right " << Right.x << " " << Right.y << " " <<Right.z << " " << Right.w << endl;
    
    Bottom.x = comp[0][3] + comp[0][1]; // see handout to fill in with values from comp
    Bottom.y = comp[1][3] + comp[1][1]; // see handout to fill in with values from comp
    Bottom.z = comp[2][3] + comp[2][1]; // see handout to fill in with values from comp
    Bottom.w = comp[3][3] + comp[3][1]; // see handout to fill in with values from comp
    magnitude = sqrt(pow(Bottom.x, 2) + pow(Bottom.y, 2) + pow(Bottom.z, 2));
    Bottom.x = Bottom.x / magnitude;
    Bottom.y = Bottom.y / magnitude;
    Bottom.z = Bottom.z / magnitude;
    Bottom.w = Bottom.w / magnitude;
    planes[2] = Bottom;
    //cout << "Bottom " << Bottom.x << " " << Bottom.y << " " <<Bottom.z << " " << Bottom.w << endl;
    
    Top.x = comp[0][3] - comp[0][1]; // see handout to fill in with values from comp
    Top.y = comp[1][3] - comp[1][1]; // see handout to fill in with values from comp
    Top.z = comp[2][3] - comp[2][1]; // see handout to fill in with values from comp
    Top.w = comp[3][3] - comp[3][1]; // see handout to fill in with values from comp
    magnitude = sqrt(pow(Top.x, 2) + pow(Top.y, 2) + pow(Top.z, 2));
    Top.x = Top.x / magnitude;
    Top.y = Top.y / magnitude;
    Top.z = Top.z / magnitude;
    Top.w = Top.w / magnitude;
    planes[3] = Top;
    //cout << "Top " << Top.x << " " << Top.y << " " <<Top.z << " " << Top.w << endl;
    
    Near.x = comp[0][3] + comp[0][2]; // see handout to fill in with values from comp
    Near.y = comp[1][3] + comp[1][2]; // see handout to fill in with values from comp
    Near.z = comp[2][3] + comp[2][2]; // see handout to fill in with values from comp
    Near.w = comp[3][3] + comp[3][2]; // see handout to fill in with values from comp
    magnitude = sqrt(pow(Near.x, 2) + pow(Near.y, 2) + pow(Near.z, 2));
    Near.x = Near.x / magnitude;
    Near.y = Near.y / magnitude;
    Near.z = Near.z / magnitude;
    Near.w = Near.w / magnitude;
    planes[4] = Near;
    //cout << "Near " << Near.x << " " << Near.y << " " <<Near.z << " " << Near.w << endl;
    
    Far.x = comp[0][3] - comp[0][2]; // see handout to fill in with values from comp
    Far.y = comp[1][3] - comp[1][2]; // see handout to fill in with values from comp
    Far.z = comp[2][3] - comp[2][2]; // see handout to fill in with values from comp
    Far.w = comp[3][3] - comp[3][2]; // see handout to fill in with values from comp
    magnitude = sqrt(pow(Far.x, 2) + pow(Far.y, 2) + pow(Far.z, 2));
    Far.x = Far.x / magnitude;
    Far.y = Far.y / magnitude;
    Far.z = Far.z / magnitude;
    Far.w = Far.w / magnitude;
    planes[5] = Far;
    //cout << "Far " << Far.x << " " << Far.y << " " <<Far.z << " " << Far.w << endl;
}

/* helper function to compute distance to the plane */
float DistToPlane(float A, float B, float C, float D, vec3 point) {
    float dist = ((A * point.x) + (B * point.y) + (C * point.z) + D);
    return dist;
}

/* Actual cull on planes */
//returns 1 to CULL
int ViewFrustCull(vec3 center, float radius, bool cull) {
    float dist;
    
    if (cull) {
        //cout << "testing against all planes" << endl;
        for (int i=0; i < 6; i++) {
            //Added 4 to the distance as a buffer to fix clippling issue
            dist = DistToPlane(planes[i].x, planes[i].y, planes[i].z, planes[i].w, center) + VFC_BUF;
            //test against each plane
            if (dist < radius) {
                return 1;
            }
        }
        return 0;
    } else {
        return 0;
    }
}
