/*
  Ultrasonic Radar - Processing display (v1.1)

  Reads "angle,distance" lines from the ESP32 over serial and draws a
  military-style radar: green scan line with fading trail, calibrated rings at
  5 / 10 / 15 cm, and red points for detected objects that fade slowly after
  the scan line passes (stored in an angle memory array).
*/

import processing.serial.*;

// ---- Settings ---------------------------------------------------------------
final int BAUD_RATE    = 115200;
final int PORT_INDEX   = 0;    // index in Serial.list(); change it if the wrong port is opened
final int MAX_RANGE_CM = 15;   // must match the firmware

Serial myPort;
int iAngle = 0;
int iDistance = MAX_RANGE_CM;
int[] radarDistances = new int[181];   // one cell per degree (0..180)

void setup() {
  size(1000, 600);
  smooth();

  String[] ports = Serial.list();
  if (ports.length == 0) {
    println("Error: no serial port found. Connect the ESP32 and restart the sketch.");
  } else {
    println("Available ports:");
    printArray(ports);
    if (PORT_INDEX >= ports.length) {
      println("Error: PORT_INDEX " + PORT_INDEX + " is out of range. Edit it in the sketch.");
    } else {
      println("Connecting to: " + ports[PORT_INDEX]);
      myPort = new Serial(this, ports[PORT_INDEX], BAUD_RATE);
      myPort.bufferUntil('\n');
    }
  }

  for (int i = 0; i < radarDistances.length; i++) {
    radarDistances[i] = MAX_RANGE_CM;
  }
}

void draw() {
  // translucent black rectangle = trail fading effect
  fill(0, 20);
  noStroke();
  rect(0, 0, width, height);

  drawRadar();
  drawLine();
  drawObject();
  drawText();
}

void serialEvent(Serial port) {
  String data = port.readStringUntil('\n');
  if (data == null) return;

  data = trim(data);
  String[] parts = split(data, ',');
  if (parts.length < 2) return;

  try {
    int angle = Integer.parseInt(trim(parts[0]));
    int distance = Integer.parseInt(trim(parts[1]));

    if (angle >= 0 && angle <= 180) {
      iAngle = angle;
      iDistance = constrain(distance, 0, MAX_RANGE_CM);
      radarDistances[angle] = iDistance;
    }
  } catch (NumberFormatException e) {
    // Noise on the line: ignore the sample but keep a trace for debugging
    println("Ignored malformed line: " + data);
  }
}

void drawRadar() {
  pushMatrix();
  translate(width / 2, height - 50);
  noFill();
  strokeWeight(2);
  stroke(98, 245, 31);

  // range rings: 15 cm (outer), 10 cm, 5 cm
  arc(0, 0, width * 0.9, width * 0.9, PI, TWO_PI);
  arc(0, 0, width * 0.6, width * 0.6, PI, TWO_PI);
  arc(0, 0, width * 0.3, width * 0.3, PI, TWO_PI);

  line(-width / 2, 0, width / 2, 0);

  // angle guide lines every 30 degrees
  for (int angle = 30; angle <= 150; angle += 30) {
    float x = (width / 2 * 0.9) * cos(radians(angle));
    float y = -(width / 2 * 0.9) * sin(radians(angle));
    line(0, 0, x, y);
  }

  // ring labels
  fill(98, 245, 31);
  textSize(12);
  textAlign(LEFT, TOP);
  text("5 cm",  width * 0.15 + 4, 4);
  text("10 cm", width * 0.30 + 4, 4);
  text("15 cm", width * 0.45 + 4, 4);
  popMatrix();
}

void drawLine() {
  pushMatrix();
  translate(width / 2, height - 50);
  strokeWeight(4);
  stroke(30, 250, 60);
  float len = width * 0.45;   // same radius as the outer ring
  float x = len * cos(radians(iAngle));
  float y = -len * sin(radians(iAngle));
  line(0, 0, x, y);
  popMatrix();
}

void drawObject() {
  pushMatrix();
  translate(width / 2, height - 50);

  for (int angle = 0; angle <= 180; angle++) {
    int dist = radarDistances[angle];

    if (dist > 0 && dist < MAX_RANGE_CM) {
      // points fade as the scan line moves away from them
      int angleDiff = abs(iAngle - angle);
      float opacity = max(20, 255 - angleDiff * 5);

      strokeWeight(12);
      stroke(255, 10, 10, opacity);

      float pixelsPerCm = (width * 0.45) / MAX_RANGE_CM;
      float r = dist * pixelsPerCm;
      point(r * cos(radians(angle)), -r * sin(radians(angle)));
    }
  }
  popMatrix();
}

void drawText() {
  fill(98, 245, 31);
  textSize(18);
  textAlign(LEFT, BASELINE);
  text("Radar Scanner (ESP32) - " + MAX_RANGE_CM + " cm max range", 50, 50);
  text("Angle: " + iAngle + "\u00b0", 50, 85);
  text("Distance: " + (iDistance >= MAX_RANGE_CM ? "Out of range" : iDistance + " cm"), 50, 115);
}
