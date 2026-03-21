
void __attribute__((fastcall)) setPixel(int offset, int color);   // from assembly   (fastcall convention)

void __attribute__((fastcall)) fillScreen(void);     // from assembly   (fastcall convention)

void __attribute__((fastcall)) clearDepth(float* depthBuffer);     // from assembly   (fastcall convention)

void __attribute__((fastcall)) drawBuffer(void);       // from assembly   (fastcall convention)

float tan(float radians);                     // from assembly   (arguments passed on stack, no fastcall)






const float PI = 3.1415927410125732421875;     //   "PI" constant definition



const float PROJ_F = 1.0;            //   f = 1 / tan(fovY / 2)        USED BY PROJECTION MATRIX GENERATION,  fovY = desired vertical field-of-view



#define MAT4_ZERO {0.0f, 0.0f, 0.0f, 0.0f, \
                   0.0f, 0.0f, 0.0f, 0.0f, \
                   0.0f, 0.0f, 0.0f, 0.0f, \
                   0.0f, 0.0f, 0.0f, 0.0f}


#define WIDTH 800
#define HEIGHT 600



#include "lut.h"
#define TABLE_SIZE 4320
#define SAMPLES_PER_DEGREE 12

float remainderFunc(float x, float y) {
    if (y == 0.0f) return 0.0f; // Handle division by zero


    float sign = (x < 0) ? -1.0f : 1.0f;

    // Convert both to positive for the loop
    float abs_x = (x < 0) ? -x : x;
    float abs_y = (y < 0) ? -y : y;

    // Repeated subtraction
    while (abs_x >= abs_y) {
        abs_x -= abs_y;
    }

    // Restore original sign
    return abs_x * sign;
}


float sin(float degrees) {
    // 1. Handle negative angles (e.g., -90 should be 270)
    while (degrees < 0.0f) degrees += 360.0f;
    
    // 2. Handle wrapping (e.g., 720 should be 0)
    while (degrees >= 360.0f) degrees -= 360.0f;

    // 3. Convert degrees to array index
    int index = (int)(degrees * SAMPLES_PER_DEGREE);
    if (index < 0) index = 0;
    if (index >= TABLE_SIZE) index = TABLE_SIZE - 1;  // Changed from 0

    // 4. Safety clamp (in case of floating point rounding errors)
    if (index >= TABLE_SIZE) index = 0;

    return sine_table[index];
}

float cos(float degrees){               //  uses sine_table, but with (degrees + 90)
    
    return sin(degrees+90.0f);
}






typedef struct {      // "ColorRGB" struct

    int r;
    int g;
    int b;
} ColorRGB;


typedef struct {     // "Vertex" struct,      same as "vec3" just different name for less confusion

    float x;
    float y;
    float z;
} Vertex;


typedef struct {    //  vec3 struct,   same as "Vertex" just different name

    float x;
    float y;
    float z;
} vec3;


typedef struct {

    vec3 position;
    vec3 rotation;
    vec3 scale;
    int numPolys;    // number of polygons to render. usually just put the number of polygons in "polys"
    bool alive;
    float matrix[16];
} Object;          //  "Object" struct. has vec3 position, and rotation components, as well as


typedef struct {    // "Polygon" struct

    Vertex v1;
    Vertex v2;
    Vertex v3;
    Object* parent;
    bool alive;
} Polygon;








int rgbToHex(ColorRGB color){
    int hex = 0;
    hex |= (color.r << 16);
    hex |= (color.g << 8);
    hex |=  color.b;

    return hex;
}              




//  alloc_polys()    custom object-polygon memory allocation
//  Object must be created outside function first, then pass in pointer to the object into this function.

//  takes in:  
//  1: pointer to the object, 
//  2: an array of polygons that the object should have, 
//  3: the number of polygons, 
//  4: the polyBuffer, 
//  5: the index of the next free slot in polyBuffer not including dead polygons,
//  6: and the freePolys array
void alloc_polys(Object* object, Polygon* polys, int numPolys, Polygon* polyBuffer, int* polyBufferEnd,  int* freePolys){  
    for (int i = 0; i < numPolys; i++){
        for (int j = 0; j < 1000; j++){
            if (j == 0 && freePolys[j] == -1 && *polyBufferEnd < 1000){
                polyBuffer[*polyBufferEnd] = polys[i];
                *polyBufferEnd += 1;
                break;
            }
            if (j > 0 && freePolys[j] > -1){
                polyBuffer[freePolys[j-1]] = polys[i];
                freePolys[j-1] = -1;
                break;
            }
        }
    }
}



// takes in pointer to object, removes child polygons from polyBuffer, adds the indices of removed polygons to freePolys
void delete_object(Object* object, Polygon* polyBuffer, int* freePolys){   
    for (int i = 0; i < 1000; i++){
        if (polyBuffer[i].parent == object){
            for (int j = 0; j < 1000; j++){
                if (freePolys[j] == -1){
                    freePolys[j] = i;
                    break;
                }
            }
            polyBuffer[i].alive = false;
        }
    }
    object->alive = false;
}



//   helper function, creates a triangle
//   arg 1:  pass in pointer to a new Object created outside function
void new_triangle(Object* object, Polygon* polyBuffer, int* polyBufferEnd, int* freePolys){
    Polygon polygons[1] = {     //   {v1, v2, v3, parent object (pointer), alive (true)

        { {0.0, 1.0, 0.0}, {1.0, -1.0, 0.0}, {-1.0, -1.0, 0.0}, object, true}              //   polygon 1
        
    };

    alloc_polys(object, polygons, 1, polyBuffer, polyBufferEnd, freePolys);

}


void matVecMultiply(float* mat, float* vec, float* result){     //  also divides xyz by w
    float x = vec[0];
    float y = vec[1];
    float z = vec[2];
    float w = 1.0f;

    float tx = mat[0]*x + mat[4]*y + mat[8]*z + mat[12]*w;
    float ty = mat[1]*x + mat[5]*y + mat[9]*z + mat[13]*w;
    float tz = mat[2]*x + mat[6]*y + mat[10]*z + mat[14]*w;
    float tw = mat[3]*x + mat[7]*y + mat[11]*z + mat[15]*w;

    if (tw != 0.0f) {
        result[0] = tx / tw;
        result[1] = ty / tw;
        result[2] = tz / tw;
    } else {
        result[0] = tx;
        result[1] = ty;
        result[2] = tz;
    }
}

void mat4x4Multiply(float* a, float* b, float* result) {
    for(int row = 0; row < 4; row++) {
        for(int col = 0; col < 4; col++) {
            result[col*4 + row] = 0.0f;              
            for(int k = 0; k < 4; k++) {
                result[col*4 + row] += a[k*4 + row] * b[col*4 + k];
            }
        }
    }
}



void genPerspectiveMatrix(float near, float far, float aspect, float* array){
    float f = PROJ_F;     //1/tan((PI/180*fovy)/2.0f);
    float a = f/aspect;
    float b = f;
    float c = far/(far-near);
    float d = (-near*far)/(far-near);
    float matrix[16] = {
        a, 0, 0, 0,
        0, b, 0, 0,
        0, 0, c, -1,
        0, 0, d, 0};
    for (int i = 0; i < 16; i++){
        array[i] = matrix[i];
    }
    return;
}


void genModelMatrix(Object* object, float* modelMatrix){

    float tx = object->position.x;
    float ty = object->position.y;
    float tz = object->position.z;

    float rx = object->rotation.x;
    float ry = object->rotation.y;
    float rz = object->rotation.z;

    while (rx < 0.0f) rx += 360.0f;
    while (rx >= 360.0f) rx -= 360.0f;


    float sinx = sin(rx);
    float cosx = cos(rx);

    float siny = sin(ry);
    float cosy = cos(ry);

    float sinz = sin(rz);
    float cosz = cos(rz);

    float scalex = object->scale.x;
    float scaley = object->scale.y;
    float scalez = object->scale.z;

    float translationMatrix[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0, 
    0, 0, 1, 0, 
    tx, ty, tz, 1
    };

    float rotationX[16] = {
    1, 0, 0, 0,
    0, cosx, sinx, 0,
    0, -sinx, cosx, 0,
    0, 0, 0, 1
    };

    float rotationY[16] = {
    cosy, 0, -siny, 0,
    0, 1, 0, 0,
    siny, 0, cosy, 0,
    0, 0, 0, 1
    };

    float rotationZ[16] = {
    cosz, -sinz, 0, 0,
    sinz, cosz, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
    };

    float scaleMatrix[16] = {
    scalex, 0, 0, 0,
    0, scaley, 0, 0,
    0, 0, scalez, 0,
    0, 0, 0, 1
    };

    //float rotationMatrix[16] = rotationX;

    float tempMatrix[16];
    float tempMatrix2[16];
    float tempRotation[16];
    float rotationMatrix[16];
    mat4x4Multiply(rotationX, rotationY, tempRotation);
    mat4x4Multiply(tempRotation, rotationZ, rotationMatrix);
    mat4x4Multiply(scaleMatrix, rotationMatrix, tempMatrix2);
    mat4x4Multiply(translationMatrix, tempMatrix2, tempMatrix);

    for(int i = 0; i < 16; i++){
        modelMatrix[i] = tempMatrix[i];
    }  
}


int minOf3(int a, int b, int c){
    if (a < b){
        if (a < c){
            return a;
        }
        else{
            return c;
        }
    }
    else{
        if (b < c){
            return b;
        }
        else{
            return c;
        }
    }
}

int maxOf3(int a, int b, int c){
    if (a > b){
        if (a > c){
            return a;
        }
        else{
            return c;
        }
    }
    else{
        if (b > c){
            return b;
        }
        else{
            return c;
        }
    }
}







// shader function declarations
void shader_start(Polygon* poly, float* perspectiveMatrix, float* depthBuffer);
void shader_rasterize(float* depthBuffer, Vertex v1, Vertex v2, Vertex v3);







// shader function definitions

//   takes a pointer to a "Polygon", a 16 float array (the perspective matrix), and the depthBuffer (800*600 linear array)
void shader_start(Polygon* poly, float* perspectiveMatrix, float* depthBuffer){
    float* MP = poly->parent->matrix;

    Vertex v1 = poly->v1;
    float v1b[4] = {v1.x, v1.y, v1.z, 1.0f};
    float v1c[4];
    matVecMultiply(MP, v1b, v1c);
    v1.x = v1c[0];
    v1.y = v1c[1];
    v1.z = v1c[2];
    
    Vertex v2 = poly->v2;
    float v2b[4] = {v2.x, v2.y, v2.z, 1.0f};
    float v2c[4];
    matVecMultiply(MP, v2b, v2c);
    v2.x = v2c[0];
    v2.y = v2c[1];
    v2.z = v2c[2];
    
    Vertex v3 = poly->v3;
    float v3b[4] = {v3.x, v3.y, v3.z, 1.0f};
    float v3c[4];
    matVecMultiply(MP, v3b, v3c);
    v3.x = v3c[0];
    v3.y = v3c[1];
    v3.z = v3c[2];

    shader_rasterize(depthBuffer, v1, v2, v3);
}


void shader_rasterize(float* depthBuffer, Vertex v1, Vertex v2, Vertex v3){
    int A[2] = { (int)(v1.x*(WIDTH/2.0f)+(WIDTH/2.0f)), (int)(v1.y*-1*(HEIGHT/2.0f)+HEIGHT/2.0f)};
    int B[2] = { (int)(v2.x*(WIDTH/2.0f)+(WIDTH/2.0f)), (int)(v2.y*-1*(HEIGHT/2.0f)+HEIGHT/2.0f)};
    int C[2] = { (int)(v3.x*(WIDTH/2.0f)+(WIDTH/2.0f)), (int)(v3.y*-1*(HEIGHT/2.0f)+HEIGHT/2.0f)};

    int minX = minOf3(A[0], B[0], C[0]);
    int maxX = maxOf3(A[0], B[0], C[0]);
    int minY = minOf3(A[1], B[1], C[1]); 
    int maxY = maxOf3(A[1], B[1], C[1]);  

    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX >= WIDTH) maxX = WIDTH-1;
    if (maxY >= HEIGHT) maxY = HEIGHT-1;

    float scalarA, scalarB, scalarC;
    float vecAB[2];
    float vecCA[2];
    float vecBC[2];
    float vecD[2];

    vecAB[0] = B[0] - A[0];
    vecAB[1] = B[1] - A[1];

    vecCA[0] = A[0] - C[0];
    vecCA[1] = A[1] - C[1];

    vecBC[0] = C[0] - B[0];
    vecBC[1] = C[1] - B[1];

//  compute scalars once outside loop at x=minX, y=minX
//  scalarA function   (cross product):   a(x, y) = (AB.x*D.y + -AB.y*D.x)
//  for each increment of x (x += 1), we can just add -AB.y because the x component is mutiplied by -AB.y.
//  this can be done with y (y += 1), but add +AB.x

    vecD[0] = minX - A[0];   // get the initial value (vector from first pixel to A)
    vecD[1] = minY - A[1];
    scalarA = (vecAB[0] * vecD[1] - vecAB[1] * vecD[0]);
    
    vecD[0] = minX - B[0];  // (vector from first pixel to B)
    vecD[1] = minY - B[1];
    scalarB = (vecBC[0] * vecD[1] - vecBC[1] * vecD[0]);

    vecD[0] = minX - C[0];   // (vector from first pixel to C)
    vecD[1] = minY - C[1];
    scalarC = (vecCA[0] * vecD[1] - vecCA[1] * vecD[0]);

    ColorRGB color = {255, 0, 255};
    
    for(int y = minY; y <= maxY; y++){
        // Fresh calculation at (minX, y)
        float d0 = minX - A[0];
        float d1 = y - A[1];
        scalarA = vecAB[0] * d1 - vecAB[1] * d0;
        
        d0 = minX - B[0];
        d1 = y - B[1];
        scalarB = vecBC[0] * d1 - vecBC[1] * d0;
        
        d0 = minX - C[0];
        d1 = y - C[1];
        scalarC = vecCA[0] * d1 - vecCA[1] * d0;
        
        for(int x = minX; x <= maxX; x++){

            if( (scalarA <= 0 && scalarB <= 0 && scalarC <= 0) 
                || (scalarA >= 0 && scalarB >= 0 && scalarC >= 0) ){
                setPixel(x + y * 800, rgbToHex(color));
            }
            scalarA -= vecAB[1];
            scalarB -= vecBC[1];
            scalarC -= vecCA[1];

        }
    }


}






void bufferFlipTimer(int* frameCounter){   // called every frame, only copies backBuffer to frontBuffer at a set interval. needs a pointer to an integer for counting
    if (*frameCounter >= 1){
        drawBuffer();
        *frameCounter = -1;
    }
    *frameCounter += 1;    
}



void drawLoop(void){

}


void setModelMatrix(Object* object, float* perspectiveMatrix){
    float modelMatrix[16];
    genModelMatrix(object, modelMatrix);
    mat4x4Multiply(perspectiveMatrix, modelMatrix, object->matrix);
}

void clearScreen(float* depthBuffer){
    fillScreen();
    //clearDepth(depthBuffer);
}

//__attribute__((naked))
void __attribute__((fastcall)) c_main(float* depthBuffer) {

    int x = 0;




    int frameCounter = 0;




    //  polygon memory setup
    Polygon* polyBuffer = (Polygon*)0xA00000;

    int polyBufferEnd = 0;
    int freePolys[1000];
    for (int i = 0; i < 1000; i++){
        freePolys[i] = -1;
        polyBuffer[i].alive = false;
    }



    //  perspective matrix setup
    float perspectiveMatrix[16];
    genPerspectiveMatrix(0.1, 100.0, 800.0f / 600.0f, perspectiveMatrix);   // constant projection matrix for all objects, only need to compue once


    Object testObject = { {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, 1, true, MAT4_ZERO };   // object to be set to a triangle
    new_triangle(&testObject, polyBuffer, &polyBufferEnd, freePolys);

    ColorRGB color = {0, 200, 0};
    ColorRGB color2 = {255, 0, 0};
    while (true) {
        x += 1;
        
        clearScreen(depthBuffer);

        for (int i = 0; i < 100; i++){
            setPixel(x+i*100, rgbToHex(color));
        }

        testObject.rotation.x += 0.50f;
        testObject.position.z = 5.0f;
        testObject.scale.y = 2.2f;


        setModelMatrix(&testObject, perspectiveMatrix);
        
        for (int i = 0; i < polyBufferEnd; i++){
            setPixel(800*500, rgbToHex(color2));
            setPixel(800*0, rgbToHex(color2));
            
            if (polyBuffer[i].alive == true){
                shader_start(&polyBuffer[i], perspectiveMatrix, depthBuffer);
            }
        }

        
        bufferFlipTimer(&frameCounter);
        

    }



}


