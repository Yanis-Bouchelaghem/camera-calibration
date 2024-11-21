#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include<stdio.h>
#include "Camera.h"

using namespace cv;
using namespace std;

// Paramètres de la taille du damier
const Size CHECKERBOARD_SIZE(8, 6);  // Coins internes
const float SQUARE_SIZE = 13.0;  // en millimètres
const float distance_between_cameras = 390.0;  // en millimètres

int main() {
    // Charger les données de calibration à partir du fichier XML/YAML
    cv::FileStorage fs("calibration_data.npz", cv::FileStorage::READ);

    if (!fs.isOpened()) {
        std::cerr << "Erreur lors de l'ouverture du fichier de calibration." << std::endl;
        return -1;
    }

    cv::Mat cameraMatrix, distCoeffs;
    std::vector<cv::Mat> rvecs, tvecs;

    fs["mtx"] >> cameraMatrix;
    fs["dist"] >> distCoeffs;
    fs["rvecs"] >> rvecs;
    fs["tvecs"] >> tvecs;

    fs.release();  // Fermer le fichier

    // Afficher les matrices
    std::cout << "Matrice de la caméra:" << std::endl << cameraMatrix << std::endl;
    std::cout << "Coefficients de distorsion:" << std::endl << distCoeffs << std::endl;

    // Critère de terminaison
    TermCriteria criteria(TermCriteria::EPS + TermCriteria::MAX_ITER, 30, 0.001);
    Camera camera0(CHECKERBOARD_SIZE,
                    SQUARE_SIZE,
                    distance_between_cameras,
                    0,
                    criteria);
    Camera camera1(CHECKERBOARD_SIZE,
                    SQUARE_SIZE,
                    distance_between_cameras,
                    1,
                    criteria);
    while (true)
    {
        auto corners0 = camera0.Tick();
        auto corners1 = camera1.Tick();
        float distance_to_board = 0.f;
        if (!corners0.empty() && !corners1.empty())
        {
            // Calculer la distance euclidienne en pixels entre les points correspondants
            float pixel_distance = norm(corners0[0] - corners1[0]);
            // Calculer la distance entre la plaque et les caméras
            distance_to_board = (distance_between_cameras * SQUARE_SIZE) / pixel_distance;

        }
        camera0.Draw(corners0, distance_to_board);
        camera1.Draw(corners1, distance_to_board);
        // Appuyer sur 'q' pour quitter
        if (waitKey(1) == 'q') {
            break;
        }
    }
    return 0;
}
