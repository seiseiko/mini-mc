#ifndef NOISE_H
#define NOISE_H
#include <QList>
#include <la.h>


float fbm(int x, int y);

float interpNoise2D(float x, float y);

float noise2D(float x, float y);


float noise2D(float x, float y){
    float product = glm::dot(glm::vec2((float)x,(float)y),glm::vec2(12.9898,4.1414));
    float sin_product = glm::sin(glm::radians(product));
    double intPart;
    float result = modf(sin_product * 43758.5453, &intPart);
    return result;

}

float noise3D(float x, float y,float z){
    float product = glm::dot(glm::vec3((float)x,(float)y,(float)z),glm::vec3(12.9898,4.1414,8.6528));
    float sin_product = glm::sin(glm::radians(product));
    double intPart;
    float result = modf(sin_product * 43758.5453, &intPart);
    return result;

}

float interpNoise2D(float x, float y){
    double x_int;
    double y_int;
    double x_fract = modf(x,&x_int);
    double y_fract = modf(y,&y_int);

    float a = noise2D(x_int,y_int);
    float b = noise2D(x_int + 1.0,y_int);
    float c = noise2D(x_int,y_int + 1.0);
    float d = noise2D(x_int + 1.0,y_int + 1.0);

    float i1 = x_fract * b + (1.0 - x_fract) * a;
    float i2 = x_fract * d + (1.0 - x_fract) * c;
    float result = y_fract * i2 + (1.0 - y_fract) * i1;
    return result;
}

float fbm(int x, int y,float persistence, float freq, int octaves){
//    float total = 0;
//    float persistence = 0.2f;
//    int octaves = 5;

//    for(int i = 1; i <= octaves; i++) {
//        float freq = pow(0.11f, i);

//        float amp = pow(persistence, i);

//        total += interpNoise2D(x * freq,
//                               y * freq) * amp;
//    }
//    return total;

            float total = 0;


            for(int i = 1; i <= octaves; i++) {
                freq = pow(freq, i);

                float amp = pow(persistence, i);

                total += interpNoise2D(x * freq,
                                       y * freq) * amp;
            }
            return total;



}


#endif // NOISE_H
