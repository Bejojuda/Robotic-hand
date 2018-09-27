#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <Servo.h>

#define FREEDOM_DEGREES 2
#define MAX_SERVO_RANGE 180
#define MEDIDAS 500

void mayorMenor();
void moverDedo(int dedo);
int convert_percentage_to_pos(int percentage_value);
int convert_pos_to_freedom_degree(int pos_value);
int execute_movement(int degree_value, Servo motor_to_move);

MPU6050 mpu;
String porcentajes;
float timeStep = 0.01;
float roll = 0;
int porcentajeRoll=0;
short medidas=0;
float acumulado[6];
Servo wrist;
int contador=0;

struct dedo{
  int pin;
  int valorFlex;
  int mayor;
  int menor;
  int porcentaje;
  Servo motor;
} dedos[5];



void setup() {
  Serial.begin(9600);

  dedos[0].pin = A0;
  dedos[1].pin  = A1;
  dedos[2].pin  = A2;
  dedos[3].pin  = A3;
  dedos[4].pin  = A6;

  dedos[0].motor.attach(3);
  dedos[1].motor.attach(5);
  dedos[2].motor.attach(6);
  dedos[3].motor.attach(9);
  dedos[4].motor.attach(10);

  for(int i=0; i<5; i++){
    acumulado[i]=0;
  }
  wrist.attach(11);


  for(int i=0; i<5; i++){
    dedos[i].mayor=0;
    dedos[i].menor=300;
  }

  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("No se encuentra ningún sensor MPU6050");
    delay(500);
  }

  mpu.calibrateGyro();

  mpu.setThreshold(3);

}

void loop() {
  if(medidas==MEDIDAS){
    for(int i=0; i<5; i++){
      dedos[i].valorFlex = acumulado[i]/MEDIDAS;
      acumulado[i]=0;
    }
    medidas=0;


    Vector norm = mpu.readNormalizeGyro();
    roll = roll + norm.XAxis * timeStep;
    //roll=acumulado[5]/10;
    acumulado[5]=0;
    porcentajeRoll = (roll+60) * 100 / 120;

    int posicion = convert_percentage_to_pos(porcentajeRoll);
    int grado_libertad = convert_pos_to_freedom_degree(posicion);
    execute_movement(grado_libertad, wrist);

    mayorMenor();
    String p;
    for(int j=0; j<5; j++){
      p += String(dedos[j].porcentaje) + "% ";
    }
    Serial.println(String(p) + " roll "+roll+ " Porc "+porcentajeRoll);
    p="";
    contador++;
    if(contador==MEDIDAS/10){
      for(int i=0; i<5; i++){
        dedos[i].mayor = dedos[i].valorFlex;
        dedos[i].menor = dedos[i].valorFlex;
      }
      Serial.println("AAAAAAAAA");
      contador=0;
    }
  }
  else{
    for(int i=0; i<5; i++){
      acumulado[i]+=analogRead(dedos[i].pin);
    }

    /*Vector norm = mpu.readNormalizeGyro();
    roll = roll + norm.XAxis * timeStep;
    acumulado[5]+=roll;*/
    medidas++;
  }




  //delay(50);
}

void mayorMenor(){
  for(int i=0; i<5; i++){
    if(dedos[i].valorFlex>dedos[i].mayor)
      dedos[i].mayor = dedos[i].valorFlex;
    if(dedos[i].valorFlex<dedos[i].menor)
      dedos[i].menor= dedos[i].valorFlex;
    moverDedo(i);
  }
}

void moverDedo(int dedo){
  int mayor = dedos[dedo].mayor - dedos[dedo].menor;
  int valor = dedos[dedo].valorFlex - dedos[dedo].menor;
  dedos[dedo].porcentaje = valor * 100 / mayor;
  int posicion = convert_percentage_to_pos(dedos[dedo].porcentaje);
  execute_movement(posicion, dedos[dedo].motor);
}

int convert_percentage_to_pos(int percentage_value)
{
    int max_percentage = 100;
    return (percentage_value * MAX_SERVO_RANGE) / max_percentage;
}

int convert_pos_to_freedom_degree(int pos_value)
{
    int available_freedom_degrees = MAX_SERVO_RANGE / FREEDOM_DEGREES;
    int actual_pos = available_freedom_degrees;
    int actual_degree = 1;
    for (int i = 1; i <= FREEDOM_DEGREES; i++)
    {
        if (pos_value < actual_pos)
        {
            actual_degree = i;
            break;
        }
        else
        {
            actual_pos += available_freedom_degrees;
        }
    }
    return actual_degree;
}

//Actua solo en dos posibles posiciones
// 1: 0 grados
// 2: 180 grados
int execute_movement(int pos_to_move, Servo motor_to_move)
{
    int final_pos = 0;
    if (pos_to_move >= MAX_SERVO_RANGE / 2) {
      final_pos = MAX_SERVO_RANGE;
    }
    // int available_freedom_degrees = MAX_SERVO_RANGE / FREEDOM_DEGREES;
    // extract total position with the average value of the
    // range according to the last freedom degree
    // int pos_to_move = ((available_freedom_degrees * degree_value) - (available_freedom_degrees / 2));
    motor_to_move.write(final_pos);
    return final_pos;
}
