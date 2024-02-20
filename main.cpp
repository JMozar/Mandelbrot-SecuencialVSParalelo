#include <SFML/Graphics.hpp>
#include <thread>
#include <vector>
#include <chrono>
#include <windows.h>
#include <sstream>

//Pantalla
const int WIDTH = 600;
const int HEIGHT = 600;
const int MAX_ITERATIONS = 10000;

//Mandelbrot secuencial
int mandelbrot(double cx, double cy) {
    double x = 0.0, y = 0.0;
    int iterations = 0;
    while (x * x + y * y < 4.0 && iterations < MAX_ITERATIONS) {
        double xtemp = x * x - y * y + cx;
        y = 2 * x * y + cy; //parte imaginaria
        x = xtemp; //parte real
        iterations++;
    }
    return iterations;
}

//Mandelbrot limitado
void calculateMandelbrotRegion(int startRow, int endRow, sf::Image& image) {
    for (int i = 0; i < WIDTH; ++i) {
        for (int j = startRow; j < endRow; ++j) {
            double x0 = (i - WIDTH / 2.0) * 4.0 / WIDTH;
            double y0 = (j - HEIGHT / 2.0) * 4.0 / HEIGHT;
            int color = mandelbrot(x0, y0) % 256;
            image.setPixel(i, j, sf::Color(color, color, color));
        }
    }
}

//Codigo principal
int main() {
	
	//Creacion de ventana
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set");
    sf::Image image;
    image.create(WIDTH, HEIGHT, sf::Color::Black);
    
    //Analisis secuencial
    auto startTime = std::chrono::high_resolution_clock::now();
	calculateMandelbrotRegion(0, HEIGHT, std::ref(image));
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration_secuencial = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
	
	//Analisis paralelo
    startTime = std::chrono::high_resolution_clock::now();
    int numThreads = std::thread::hardware_concurrency();
    //numThreads = 2;
    std::vector<std::thread> threads(numThreads);

	//Particionamiento
    int rowsPerThread = HEIGHT / numThreads;
    int startRow = 0;
    int endRow = 0;
	
	//Mandelbrot paralelo
    for (int i = 0; i < numThreads; ++i) {
        endRow = startRow + rowsPerThread;
        if (i == numThreads - 1) {
            endRow = HEIGHT;
        }
        threads[i] = std::thread(calculateMandelbrotRegion, startRow, endRow, std::ref(image));
        startRow = endRow;
    }
    
    for (int i = 0; i < numThreads; ++i) {
        threads[i].join();
    }
    endTime = std::chrono::high_resolution_clock::now();
    auto duration_paralelo = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

	//Ventana con la figura
    sf::Texture texture;
    texture.loadFromImage(image);
    sf::Sprite sprite(texture);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        window.clear();
        window.draw(sprite);
        window.display();
    }

    // Ventana emergente con los resultados
	float Speedup=(float)duration_secuencial/duration_paralelo;
    std::ostringstream oss;
    oss << "Hilos: " << numThreads << " \n"
		<< "Tiempo secuencial: " << duration_secuencial << "ms \n"
        << "Tiempo paralelo: " << duration_paralelo << "ms \n"
        << "Speedup: " <<Speedup<< " \n"
		<< "Efficiency: " <<100*Speedup/numThreads<< "% \n";
    std::string contenido = oss.str();
    MessageBox(NULL, contenido.c_str(), "Resultados obtenidos", MB_OK | MB_ICONINFORMATION);
    return 0;
}

