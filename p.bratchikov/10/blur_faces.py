import cv2

cap = cv2.VideoCapture(0)
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")

while True:
    ret, frame = cap.read()

    if not ret:
        break

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray = cv2.bilateralFilter(gray, 5, 75, 75)
    clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8,8))
    gray = clahe.apply(gray)

    faces = face_cascade.detectMultiScale(gray, scaleFactor=1.1, minNeighbors=5, minSize=(5, 5))

    for (x, y, w, h) in faces:
        tmp_frame = frame[y:y+h, x:x+w]
        kernel = (w // 2 * 2 - 1, h // 2 * 2 - 1)
        tmp_frame = cv2.GaussianBlur(tmp_frame, kernel, 0)
        frame[y:y+h, x:x+w] = tmp_frame

    cv2.imshow("Camera", frame)

    if cv2.waitKey(1) & 0xFF == ord('e'):
        break

cap.release()
cv2.destroyAllWindows()
