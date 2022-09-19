import cv2

filename = "d.jpg" #images

img = cv2.imread(filename,cv2.IMREAD_GRAYSCALE) # Read image via grayscale
laplacian_var = cv2.Laplacian(img, cv2.CV_64F).var() # Read in laplacian value

print(laplacian_var)

cv2.imshow("Img", img)
cv2.waitKey(0)