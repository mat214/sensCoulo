// Signal K application Coulometre.
//
// Matthias Perez matthias.perez@posteo.net

#include "math.h"

#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/lambda_transform.h"
#include "sensesp_app_builder.h"
#include "sevedirect/sensors/vedirect.h"
#include "sensesp/signalk/signalk_value_listener.h"
// #include "sensesp\system\configurable.h"
#include "ui_configurables.h"

// onewire température
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 15
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//ina226
#include <Wire.h>
#include <INA226.h>


// constante
const int t_callback = 100; // conf de la durée de la boucles courrent en milliseconde 


// variable configurable
IntConfig *CapaNominal;// Capacité batterie à renseigner en Ah ici ou dans le WebUI
FloatConfig *ChargeEfficiencyFactor;// Efficience de charge en % ou dans le WebUI
FloatConfig *Coef;//  coef de Peukert ou dans le WebUI
IntConfig *CT; //temps décharge

//variable global
int CapNomiPeuk;
bool premierDemarage = true;
float PourCharge = 100;

double Cap;
float v = 12;
INA226 ina(Wire);


using namespace sensesp;

reactesp::ReactESP app;

//fonction retour température
float requestTemperatures_callback() {
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  return temperatureC;
}

// fonction callback de retour de la capacité

// fonction callback de retour le % de charge
float read_pourCharge_callback() {
  return(PourCharge);
}

// fonction amp callback avec 
float read_amp_callback() {
  return (ina.readShuntCurrent()); 
}

float read_volt_callback() {
float Vi = v;
v = ina.readBusVoltage();
/**
if ((Vi > 14.55) and (v > 13.50) and (v < 13.85)) {
Cap = CapNomiPeuk;  
} 
*/
return v;
}


// fonction lambda lancement calcul charge décharge
auto amp_to_cap_function = [](float courant) ->float {
     
     if (premierDemarage) {
        Cap = CapNomiPeuk;
        premierDemarage = false;
     }
     
        float t = 3600000.0 / t_callback;
        if (courant > 0) {    
            Cap = Cap + (courant * (ChargeEfficiencyFactor->get_value() / 100) / t);
        } else {
            float c = - courant;
            float pk =  Coef->get_value();

            Cap = Cap - (pow(c, pk) / t);
        }
      if (Cap > CapNomiPeuk) {Cap = CapNomiPeuk;}
        PourCharge = Cap / CapNomiPeuk * 100;  
    
return (Cap);
};


// courant consomation circuit
auto lambada_courant_circuit = [](float i) ->float {
i = (- i ) - ina.readShuntCurrent();
return (i);
};




auto lecture_etat = [](String etat)->void {
if ((etat = "float") or (etat = "Float")) {
  Cap = CapNomiPeuk;  
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

  // fonction setup température
  sensors.begin();

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
CT = new IntConfig(20, "/Configuration/CT", "Temps de décharge donnée constructeur K100 = 100, K20 = 20, K5 = 5", 100);

// définition de la capacité de Peukert
CapNomiPeuk = CT->get_value() *(pow((CapaNominal->get_value()/CT->get_value()),(Coef->get_value())));


  // Sensor lié à la meusure INA
  auto* bat_current = new RepeatSensor<float>(t_callback, read_amp_callback);

  bat_current->connect_to(new SKOutputFloat("electrical.battery.current", new SKMetadata("A", "Batterie courant")));

  bat_current->connect_to(new LambdaTransform<float, float>(amp_to_cap_function))
     ->connect_to(new SKOutputFloat("electrical.battery.capacity.remaining", new SKMetadata("Ah", "Batterie capacitée restante")));

  auto* bat_pour = new RepeatSensor<float>(1000, read_pourCharge_callback);
  bat_pour->connect_to(new SKOutputFloat("electrical.battery.capacity.stateOfCharge", new SKMetadata("%", "Pourcentage capacitée")));

  auto* Bat_volt = new RepeatSensor<float>(1000, read_volt_callback);
  Bat_volt->connect_to(new SKOutputFloat(
      "electrical.battery.voltage", new SKMetadata("V", "Batterie voltage")));
   
  // connexion signalk 
  // etat de charge pour rénitialisation
  auto* skModeCharge =  new SKValueListener<String>("electrical.charger.batterie.chargingMode", 2000);
  skModeCharge->connect_to(new LambdaConsumer<String>(lecture_etat));
 

  // LambaTransform courant circuit courant chargeur  - courant baterie
  auto* skCharge =  new SKValueListener<float>("electrical.solar.victronDevice.loadCurrent", 1000);
  skCharge->connect_to(new LambdaTransform<float, float>(lambada_courant_circuit))
    ->connect_to(new SKOutputFloat("electrical.circuit.current", new SKMetadata("A", "Circuit courant")));



  // sensor température
   auto* bat_temp = new RepeatSensor<float>(t_callback, requestTemperatures_callback);
   bat_temp->connect_to(new SKOutputFloat("electrical.battery.temperature", new SKMetadata("C", "batterie température")));

  // fonction sensESP
  sensesp_app->start(); 
}

// loop lié à sensESP
void loop() { app.tick(); }