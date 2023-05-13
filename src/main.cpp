// Signal K application Coulometre.
//
// Matthias Perez matthias.perez@posteo.net


#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/lambda_transform.h"
#include "sensesp_app_builder.h"
#include "sevedirect/sensors/vedirect.h"
// #include "sensesp\system\configurable.h"
#include "ui_configurables.h"


// NOTE: IMPORTANT! Victron MPPT chargers use a 5V VE.Direct interface, so
// a logic level converter or a voltage divider MUST be used to interface the
// devices. You _will_ damage the ESP32 device is you fail to do so!

#include <Wire.h>
#include <INA226.h>


// constante
const int t_callback = 100; // conf de la durée de la boucles courrent en milliseconde 
constexpr int kTXPin = 13; // Pin TX de l'INA
constexpr int kRXPin = 12; // Pin RX de l'INA

// variable configurable
IntConfig *CapaNominal;// Capacité batterie à renseigner en Ah ici ou dans le WebUI
FloatConfig *ChargeEfficiencyFactor;// Efficience de charge en % ou dans le WebUI
FloatConfig *Coef;//  coef de Peukert ou dans le WebUI

//variable global
bool premierDemarage = true;
float PourCharge = 100;
String EtatCharge;
double Cap;
INA226 ina(Wire);

// Provide an ID for the charge controller, to be used in the Signal K
// and the configuration paths
#define SOLAR_CHARGE_CONTROLLER_ID "1"

using namespace sensesp;

reactesp::ReactESP app;

// fonction callback de retour de la capacité

// fonction callback de retour le % de charge
float read_pourCharge_callback() {
  return(PourCharge);
}

// fonction amp callback avec 
float read_amp_callback() {
  return (- ina.readShuntCurrent()); 
}
// fonction lambda lancement calcul charge décharge
auto amp_to_cap_function = [](float courant) ->float {
     
     if (premierDemarage) {
        Cap = CapaNominal->get_value();
        premierDemarage = false;
     }
     if (EtatCharge != "Float") {
        float t = 3600000 / t_callback;
        if (courant > 0) {    
            Cap = Cap + (courant * (ChargeEfficiencyFactor->get_value() / 100) / t);
        } else {    
            Cap = Cap - (pow(-courant,Coef->get_value()) / t);
        }
        PourCharge = Cap / CapaNominal->get_value() * 100;  
    }
return (Cap);
};

//********** bug a vérifier donnée incohérente !!!!! ***************
// courant consomation circuit
auto lambada_courant_circuit = [](float i) ->float {
i = (- i * 1000) - ina.readShuntCurrent();
return (i);
};

// traduction int soc en string et initialisation au float
auto Etat_text = [](int soc) ->String {
    
    switch( soc ) {
              case 0:
                  return EtatCharge = "Off";
              case 2:
                return EtatCharge ="Fault";
              case 3:
                  return EtatCharge ="Bulk";
              case 5:
                  Cap = CapaNominal->get_value();
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
  
  // variable configurable  UI batterie

CapaNominal = new IntConfig(70, "/Configuration/Capacité Batérie", "en Ah", 100);
Coef = new FloatConfig(1.24, "/Configuration/Coef de Peukert","", 100);
ChargeEfficiencyFactor = new FloatConfig(90, "/Configuration/Efficience de charge","en %", 100);
  // initialize Serial1 INA on the opto_in pin
  Serial1.begin(19200, SERIAL_8N1, kRXPin, kTXPin, false);

  // Flux VEDirect MPTT
  VEDirectInput* vedi = new VEDirectInput(&Serial1);
  vedi->parser.data.channel_1_battery_voltage.connect_to(new SKOutputFloat(
      "electrical.battery." SOLAR_CHARGE_CONTROLLER_ID ".voltage", new SKMetadata("V", "Batterie voltage")));
  vedi->parser.data.channel_1_battery_current.connect_to(new SKOutputFloat(
      "electrical.solar." SOLAR_CHARGE_CONTROLLER_ID ".current", new SKMetadata("mA", "Chargeur courant")));
  vedi->parser.data.load_current.connect_to(new SKOutputFloat(
      "electrical.solar." SOLAR_CHARGE_CONTROLLER_ID ".panelCurrent", new SKMetadata("mA", "Panneau courant")));
  vedi->parser.data.panel_voltage.connect_to(new SKOutputFloat(
      "electrical.solar." SOLAR_CHARGE_CONTROLLER_ID ".panelVoltage", new SKMetadata("V", "Panneau voltage")));
  vedi->parser.data.panel_power.connect_to(new SKOutputFloat(
      "electrical.solar." SOLAR_CHARGE_CONTROLLER_ID ".panelPower", new SKMetadata("W", "Panneau puissance")));
  vedi->parser.data.yield_today.connect_to(new SKOutputFloat(
      "electrical.solar." SOLAR_CHARGE_CONTROLLER_ID ".yieldToday", new SKMetadata("Wh", "Panneau Wh jour")));
  vedi->parser.data.maximum_power_today.connect_to(new SKOutputFloat(
      "electrical.solar." SOLAR_CHARGE_CONTROLLER_ID ".maxPowerToday", new SKMetadata("W", "Panneau max puissance")));

  // LambaTransform courant circuit courant chargeur  - courant baterie
  vedi->parser.data.channel_1_battery_current.connect_to(new LambdaTransform<float, float>(lambada_courant_circuit))
    ->connect_to(new SKOutputFloat("Circuit courant"));

  // Sensor lié à la meusure INA
  auto* bat_current = new RepeatSensor<float>(t_callback, read_amp_callback);

  bat_current->connect_to(new SKOutputFloat("electrical.battery." SOLAR_CHARGE_CONTROLLER_ID ".current", new SKMetadata("A", "Batterie courant")));

  bat_current->connect_to(new LambdaTransform<float, float>(amp_to_cap_function))
     ->connect_to(new SKOutputFloat("electrical.battery." SOLAR_CHARGE_CONTROLLER_ID ".capacity.remaining", new SKMetadata("Ah", "Batterie capacitée restante")));

  auto* bat_pour = new RepeatSensor<float>(1000, read_pourCharge_callback);
  bat_pour->connect_to(new SKOutputFloat("electrical.battery." SOLAR_CHARGE_CONTROLLER_ID ".capacity.stateOfCharge", new SKMetadata("%", "Pourcentage capacitée")));

  // LambaTransform du numéro soc ve.direct en txt + initialisation capacité bat au float
  vedi->parser.data.state_of_operation.connect_to(new LambdaTransform<int, String>(Etat_text))
      ->connect_to(new SKOutputString("electrical.solar." SOLAR_CHARGE_CONTROLLER_ID ".chargingMode", new SKMetadata("", "Mode de charge")));

  // fonction sensESP
  sensesp_app->start(); 
}

// loop lié à sensESP
void loop() { app.tick(); }