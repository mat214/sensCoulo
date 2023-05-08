// Signal K application Coulometre.
//
// Matthias Perez matthias.perez@posteo.net


#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/lambda_transform.h"
#include "sensesp_app_builder.h"
#include "sevedirect/sensors/vedirect.h"


// NOTE: IMPORTANT! Victron MPPT chargers use a 5V VE.Direct interface, so
// a logic level converter or a voltage divider MUST be used to interface the
// devices. You _will_ damage the ESP32 device is you fail to do so!

#include <Wire.h>
#include <INA226.h>

const int t_callback = 100; // conf de la durée de la boucles courrent en milliseconde 
INA226 ina(Wire);

constexpr int kTXPin = 13; // Pin TX de l'INA
constexpr int kRXPin = 12; // Pin RX de l'INA

const int CapNomi = 70; // capacité batterie à renseigner en Ah
double Cap = CapNomi;
float PourCharge = 100;
const float ChargeEfficiencyFactor = 0.9; // efficience de charge 
const float Coef = 1.24; // coef de Peukert
String EtatCharge;

// Provide an ID for the charge controller, to be used in the Signal K
// and the configuration paths
#define SOLAR_CHARGE_CONTROLLER_ID "1"

using namespace sensesp;

reactesp::ReactESP app;

// fonction callback de retour de la capacité
float read_cap_callback() {
  return(Cap);
}

// fonction callback de retour le % de charge
float read_pourCharge_callback() {
  return(PourCharge);
}

// fonction amp callback avec lancement calcul charge décharge
float read_amp_callback() {
  double a = ina.readShuntCurrent();
  if (EtatCharge != "Float") {
    a = -a;
    double t = t_callback / 3600000;
    if (a > 0) {    
        Cap = Cap + (a * ChargeEfficiencyFactor / 36000);
    } else {    
        Cap = Cap - (pow(-a,Coef) / 36000);
    }
    PourCharge = Cap / CapNomi * 100; 
  }
  return (a); 
  }

// traduction int soc en string et initialisation au float
auto int_soc = [](int soc) ->String {
    
    switch( soc ) {
              case 0:
                  return EtatCharge = "Off";
              case 2:
                return EtatCharge ="Fault";
              case 3:
                  return EtatCharge ="Bulk";
              case 5:
                  Cap = CapNomi;
                  return EtatCharge = "Float";
              case 6:
                  return EtatCharge = "Equalize  (manual)";
              case 245:
                  return EtatCharge = "Starting-up";
              case 247:
                  return EtatCharge = "Auto equalize / Recondition";
              case 252:
                  return EtatCharge = "External Control";
              default:
                  return EtatCharge = "NA";
          }
};


// check config INA
void checkConfig()
{
  Serial.print("Mode:                  ");
  switch (ina.getMode())
  {
    case INA226_MODE_POWER_DOWN:      Serial.println("Power-Down"); break;
    case INA226_MODE_SHUNT_TRIG:      Serial.println("Shunt Voltage, Triggered"); break;
    case INA226_MODE_BUS_TRIG:        Serial.println("Bus Voltage, Triggered"); break;
    case INA226_MODE_SHUNT_BUS_TRIG:  Serial.println("Shunt and Bus, Triggered"); break;
    case INA226_MODE_ADC_OFF:         Serial.println("ADC Off"); break;
    case INA226_MODE_SHUNT_CONT:      Serial.println("Shunt Voltage, Continuous"); break;
    case INA226_MODE_BUS_CONT:        Serial.println("Bus Voltage, Continuous"); break;
    case INA226_MODE_SHUNT_BUS_CONT:  Serial.println("Shunt and Bus, Continuous"); break;
    default: Serial.println("unknown");
  }
  
  Serial.print("Samples average:       ");
  switch (ina.getAverages())
  {
    case INA226_AVERAGES_1:           Serial.println("1 sample"); break;
    case INA226_AVERAGES_4:           Serial.println("4 samples"); break;
    case INA226_AVERAGES_16:          Serial.println("16 samples"); break;
    case INA226_AVERAGES_64:          Serial.println("64 samples"); break;
    case INA226_AVERAGES_128:         Serial.println("128 samples"); break;
    case INA226_AVERAGES_256:         Serial.println("256 samples"); break;
    case INA226_AVERAGES_512:         Serial.println("512 samples"); break;
    case INA226_AVERAGES_1024:        Serial.println("1024 samples"); break;
    default: Serial.println("unknown");
  }

  Serial.print("Bus conversion time:   ");
  switch (ina.getBusConversionTime())
  {
    case INA226_BUS_CONV_TIME_140US:  Serial.println("140uS"); break;
    case INA226_BUS_CONV_TIME_204US:  Serial.println("204uS"); break;
    case INA226_BUS_CONV_TIME_332US:  Serial.println("332uS"); break;
    case INA226_BUS_CONV_TIME_588US:  Serial.println("558uS"); break;
    case INA226_BUS_CONV_TIME_1100US: Serial.println("1.100ms"); break;
    case INA226_BUS_CONV_TIME_2116US: Serial.println("2.116ms"); break;
    case INA226_BUS_CONV_TIME_4156US: Serial.println("4.156ms"); break;
    case INA226_BUS_CONV_TIME_8244US: Serial.println("8.244ms"); break;
    default: Serial.println("unknown");
  }

  Serial.print("Shunt conversion time: ");
  switch (ina.getShuntConversionTime())
  {
    case INA226_SHUNT_CONV_TIME_140US:  Serial.println("140uS"); break;
    case INA226_SHUNT_CONV_TIME_204US:  Serial.println("204uS"); break;
    case INA226_SHUNT_CONV_TIME_332US:  Serial.println("332uS"); break;
    case INA226_SHUNT_CONV_TIME_588US:  Serial.println("558uS"); break;
    case INA226_SHUNT_CONV_TIME_1100US: Serial.println("1.100ms"); break;
    case INA226_SHUNT_CONV_TIME_2116US: Serial.println("2.116ms"); break;
    case INA226_SHUNT_CONV_TIME_4156US: Serial.println("4.156ms"); break;
    case INA226_SHUNT_CONV_TIME_8244US: Serial.println("8.244ms"); break;
    default: Serial.println("unknown");
  }
  
  Serial.print("Max possible current:  ");
  Serial.print(ina.getMaxPossibleCurrent());
  Serial.println(" A");

  Serial.print("Max current:           ");
  Serial.print(ina.getMaxCurrent());
  Serial.println(" A");

  Serial.print("Max shunt voltage:     ");
  Serial.print(ina.getMaxShuntVoltage());
  Serial.println(" V");

  Serial.print("Max power:             ");
  Serial.print(ina.getMaxPower());
  Serial.println(" W");
}

// The setup function INA performs one-time application initialization.
void setup() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

 Serial.println("Initialize INA226");
  Serial.println("-----------------------------------------------");

  Wire.begin();

  // Default INA226 address is 0x40
  bool success = ina.begin();

  // Check if the connection was successful, stop if not
  if(!success)
  {
    Serial.println("Connection error");
    while(1);
  }

  // Configure INA226
  ina.configure(INA226_AVERAGES_1, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);

  // Calibrate INA226. Rshunt = 0.0025 ohm, Max excepted current = 30A ( A modifier en fonction du shunt)
  ina.calibrate(0.0025, 30);

  // Display configuration
  checkConfig();

  Serial.println("-----------------------------------------------");

  // Construct the global SensESPApp() object
  SensESPAppBuilder builder;
  sensesp_app = (&builder)
                    // Set a custom hostname for the app.
                    ->set_hostname("senscoulo")
                    // OTA obligatoire car impossible en serie avec l'optocoupleur bug ...
                    ->enable_ota("password")
                    ->get_app();

  // initialize Serial1 INA on the opto_in pin
  Serial1.begin(19200, SERIAL_8N1, kRXPin, kTXPin, false);

  // Création de l'objet VEDirect
  VEDirectInput* vedi = new VEDirectInput(&Serial1);

  vedi->parser.data.channel_1_battery_voltage.connect_to(new SKOutputFloat(
      "electrical.battery" SOLAR_CHARGE_CONTROLLER_ID ".voltage",
      "/Signal K/battery " SOLAR_CHARGE_CONTROLLER_ID " Voltage"));
  vedi->parser.data.channel_1_battery_current.connect_to(new SKOutputFloat(
      "electrical.solar." SOLAR_CHARGE_CONTROLLER_ID ".current",
      "/Signal K/Solar Charger " SOLAR_CHARGE_CONTROLLER_ID " Current"));
  vedi->parser.data.panel_voltage.connect_to(new SKOutputFloat(
      "electrical.solar." SOLAR_CHARGE_CONTROLLER_ID ".panelVoltage",
      "/Signal K/Solar Charger " SOLAR_CHARGE_CONTROLLER_ID " Panel Voltage"));
  vedi->parser.data.panel_power.connect_to(new SKOutputFloat(
      "electrical.solar." SOLAR_CHARGE_CONTROLLER_ID ".panelPower",
      "/Signal K/Solar Charger " SOLAR_CHARGE_CONTROLLER_ID " Panel Power"));
  vedi->parser.data.yield_total.connect_to(new SKOutputFloat(
      "electrical.solar." SOLAR_CHARGE_CONTROLLER_ID ".yieldTotal",
      "/Signal K/Solar Charger " SOLAR_CHARGE_CONTROLLER_ID " Yield Total"));
  vedi->parser.data.yield_today.connect_to(new SKOutputFloat(
      "electrical.solar." SOLAR_CHARGE_CONTROLLER_ID ".yieldToday",
      "/Signal K/Solar Charger " SOLAR_CHARGE_CONTROLLER_ID " Yield Today"));
  vedi->parser.data.maximum_power_today.connect_to(new SKOutputFloat(
      "electrical.solar." SOLAR_CHARGE_CONTROLLER_ID ".maxPowerToday",
      "/Signal K/Solar Charger " SOLAR_CHARGE_CONTROLLER_ID
      " Max Power Today"));


// LambaTransform du numéro soc ve.direct en txt + initialisation capacité bat au float
auto int_to_soc_text = new LambdaTransform<int, String>(int_soc);
 
vedi->parser.data.state_of_operation.connect_to(int_to_soc_text)->connect_to(new SKOutputString(
"electrical.solar." SOLAR_CHARGE_CONTROLLER_ID ".state_of_operation",
"/Signal K/Solar Charger " SOLAR_CHARGE_CONTROLLER_ID " state of operation"));

// Sensor lié à la meusure INA
auto* bat_current = new RepeatSensor<float>(t_callback, read_amp_callback);
bat_current->connect_to(new SKOutputFloat("electrical.battery." SOLAR_CHARGE_CONTROLLER_ID ".current", new SKMetadata("A",                     
                   "battery current")));

auto* bat_cap = new RepeatSensor<float>(500, read_cap_callback);
bat_cap->connect_to(new SKOutputFloat("electrical.battery." SOLAR_CHARGE_CONTROLLER_ID ".cap", new SKMetadata("Ah",                     
                   "battery capacitée")));

auto* bat_pour = new RepeatSensor<float>(500, read_pourCharge_callback);
bat_pour->connect_to(new SKOutputFloat("electrical.battery." SOLAR_CHARGE_CONTROLLER_ID ".pourcentage", new SKMetadata("%",                     
                   "Pourcentage capacité")));

// fonction sensESP
  sensesp_app->start();
  
}

// loop lié à sensESP
void loop() { app.tick(); }