#include <iostream>
#include <fstream>
#include <string>

#include "Renderer.h"


static std::vector<float> generateGridNDC(int numCellsX, int numCellsY) {
    std::vector<float> vertices;

    // Calculate the step size in NDC
    float stepX = 2.0f / numCellsX; // Grid spans [-1, 1] (2.0 total)
    float stepY = 2.0f / numCellsY; // Grid spans [-1, 1] (2.0 total)

    // Vertical lines
    for (int i = 0; i <= numCellsX; ++i) {
        float x = -1.0f + i * stepX;
        vertices.push_back(x); vertices.push_back(-1.0f); // Start point
        vertices.push_back(x); vertices.push_back(1.0f);  // End point
    }

    // Horizontal lines
    for (int j = 0; j <= numCellsY; ++j) {
        float y = -1.0f + j * stepY;
        vertices.push_back(-1.0f); vertices.push_back(y); // Start point
        vertices.push_back(1.0f); vertices.push_back(y);  // End point
    }

    return vertices;
}


int main(void)
{
    const int Nx = 100;
    const int Ny = 100;

    float data1[Nx * Ny];
    float data2[Nx * Ny];

    std::vector<float> lines = generateGridNDC(100, 100);

    Renderer renderer(Nx, Ny, std::vector<float*>{data1, data2}, lines);

    for (int i = 0; i < Nx; ++i) {
        for (int j = 0; j < Ny; ++j) {
            data1[j + i * Ny] = (float)i * j / (float)(Nx*Ny);
            data2[j + i * Ny] = 1.;
        }
    }

    int counter = 0;
    /* Loop until the user closes the window */
    while (renderer.alive()){

        // Update data
        for (int i = 0; i < Nx; ++i) {
            for (int j = 0; j < Ny; ++j) {
                data1[j + i * Ny] = (float)i * j * cos(float(counter) / 100.) / (float)(Nx * Ny);
            }
        }
        counter += 1;

        renderer.update();
        renderer.draw();
    }

    renderer.close();

    return 0;
}