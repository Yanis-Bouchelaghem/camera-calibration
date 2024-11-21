import numpy as np
import cv2 as cv
import os
import glob
import matplotlib.pyplot as plt

def calibrate(showPics=True):
    # Read Image
    root = os.getcwd()
    calibrationDir = os.path.join(root, 'redim')
    imgPathList = glob.glob(os.path.join(calibrationDir, '*.[jJ][pP][gG]'))

    if not imgPathList:
        print("Error: No images found in the calibration directory.")
        return None, None

    # Initialize
    nRows, nCols = 9, 9
    termCriteria = (cv.TERM_CRITERIA_EPS + cv.TERM_CRITERIA_MAX_ITER, 30, 0.001)
    worldPtsCur = np.zeros((nRows * nCols, 3), np.float32)
    worldPtsCur[:, :2] = np.mgrid[0:nCols, 0:nRows].T.reshape(-1, 2)
    worldPtsList, imgPtsList = [], []

    # Find Corners
    for curImgPath in imgPathList:
        imgBGR = cv.imread(curImgPath)
        if imgBGR is None:
            print(f"Warning: Could not read image at path {curImgPath}. Please check if the file path contains special characters.")
            continue

        imgGray = cv.cvtColor(imgBGR, cv.COLOR_BGR2GRAY)
        # Try different flags to improve corner detection for non-standard orientations
        cornersFound, cornersOrg = cv.findChessboardCorners(imgGray, (nCols, nRows), cv.CALIB_CB_ADAPTIVE_THRESH + cv.CALIB_CB_NORMALIZE_IMAGE)

        if cornersFound:
            worldPtsList.append(worldPtsCur)
            cornersRefined = cv.cornerSubPix(imgGray, cornersOrg, (11, 11), (-1, -1), termCriteria)
            imgPtsList.append(cornersRefined)

            if showPics:
                cv.drawChessboardCorners(imgBGR, (nCols, nRows), cornersRefined, cornersFound)
                cv.imshow('Chessboard Corners', imgBGR)
                cv.waitKey(500)

    cv.destroyAllWindows()

    if not worldPtsList or not imgPtsList:
        print("Error: No valid chessboard corners found in any images.")
        return None, None

    # Calibrate
    ret, camMatrix, distCoeff, rvecs, tvecs = cv.calibrateCamera(worldPtsList, imgPtsList, imgGray.shape[::-1], None, None)
    print("Camera Matrix:\n", camMatrix)
    print("Reproj Error (pixels): {:.4f}".format(ret))

    # Save Calibration Parameters
    paramPath = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'calibration.npz')
    np.savez(paramPath, repError=ret, camMatrix=camMatrix, distCoeff=distCoeff, rvecs=rvecs, tvecs=tvecs)

    return camMatrix, distCoeff

def removeDistortion(camMatrix, distCoeff):
    if camMatrix is None or distCoeff is None:
        print("Error: Invalid camera matrix or distortion coefficients.")
        return

    root = os.getcwd()
    imgPath = os.path.join(root, 'test/tstez.jpg')
    img = cv.imread(imgPath)

    if img is None:
        print("Error: Could not read image for distortion removal.")
        return

    height, width = img.shape[:2]
    camMatrixNew, roi = cv.getOptimalNewCameraMatrix(camMatrix, distCoeff, (width, height), 1, (width, height))
    imgUndist = cv.undistort(img, camMatrix, distCoeff, None, camMatrixNew)

    # Draw Line to See Distortion Change
    cv.line(img, (1769, 103), (1780, 922), (255, 255, 255), 2)
    cv.line(imgUndist, (1769, 103), (1780, 922), (255, 255, 255), 2)

    plt.figure()
    plt.subplot(121)
    plt.imshow(cv.cvtColor(img, cv.COLOR_BGR2RGB))
    plt.title('Original Image')
    plt.subplot(122)
    plt.imshow(cv.cvtColor(imgUndist, cv.COLOR_BGR2RGB))
    plt.title('Undistorted Image')
    plt.show()

def runCalibration():
    calibrate(showPics=True)

def runRemoveDistortion():
    camMatrix, distCoeff = calibrate(showPics=False)
    if camMatrix is not None and distCoeff is not None:
        removeDistortion(camMatrix, distCoeff)

if __name__ == '__main__':
    runCalibration()
    #runRemoveDistortion()