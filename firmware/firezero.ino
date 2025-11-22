#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>


const char* ssid = "Wokwi-GUEST";
const char* password = "";
//manish to configure hardware
#define PIR_PIN 13
#define MQ2_PIN 34
#define DS18B20_PIN 4

WebServer server(80);


struct SensorData {
  int pir;
  int gas;
  float temp;
  String fireRisk;
};

SensorData readSensors() { 
  SensorData data;


  data.pir = random(0,2);


  data.gas = random(100, 401);

  
  data.temp = random(20, 71);

  
  if (data.gas > 300 && data.temp > 50) data.fireRisk = "HIGH";
  else if (data.gas > 200) data.fireRisk = "MEDIUM";
  else data.fireRisk = "LOW";

  return data;
}


String sensorDataToWebJSON(SensorData data) {
  String json = "{";
  json += "\"pir\": " + String(data.pir) + ",";
  json += "\"gas\": " + String(data.gas) + ",";
  json += "\"temp\": " + String(data.temp) + ",";
  json += "\"fireRisk\": \"" + data.fireRisk + "\"";
  json += "}";
  return json;
}

String sensorDataToWebhookJSON(SensorData data) {
  String json = "{";
  json += "\"pir\": " + String(data.pir) + ",";
  json += "\"gas\": " + String(data.gas) + ",";
  json += "\"temp\": " + String(data.temp) + ",";
  json += "\"fireRisk\": \"" + data.fireRisk + "\"";
  json += "}";
  return json;
}


void sendToWebhook(SensorData data) {
  HTTPClient http;

  http.begin("https://webhook.site/f702b84d-e832-40b3-9411-d175ef3f669e"); 
  http.addHeader("Content-Type", "application/json");

  String payload = sensorDataToWebhookJSON(data);
  int status = http.POST(payload);
  Serial.print("Webhook Status: "); Serial.println(status);
  http.end();
}

// sridevi part html csss
void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
<title>FireZero AI Dashboard</title>
<style>
body { font-family: Arial; background:#111; color:white; text-align:center; }
.card { background:#222; margin:20px; padding:20px; border-radius:12px; transition:0.5s; }
canvas { background:#222; border-radius:12px; margin-top:20px; }
#alertPopup {
  display:none; position: fixed; top: 20px; left: 50%;
  transform: translateX(-50%);
  background:red; color:white; padding: 15px 30px;
  border-radius: 10px; font-size: 20px; font-weight: bold;
  z-index: 100;
  animation: flash 1s infinite;
}
@keyframes flash { 0%, 50%, 100% {opacity:1;} 25%,75%{opacity:0;} }
</style>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<script>
let tempData=[], gasData=[], labels=[];
const tempCtx = document.getElementById('tempChart').getContext('2d');
const gasCtx = document.getElementById('gasChart').getContext('2d');
const tempChart = new Chart(tempCtx, {type:'line', data:{labels:labels,datasets:[{label:'Temperature (°C)',data:tempData,borderColor:'red',fill:false,tension:0.3}]}, options:{scales:{x:{title:{display:true,text:'Time'}},y:{beginAtZero:true}}}});
const gasChart = new Chart(gasCtx, {type:'line', data:{labels:labels,datasets:[{label:'Gas Value',data:gasData,borderColor:'yellow',fill:false,tension:0.3}]}, options:{scales:{x:{title:{display:true,text:'Time'}},y:{beginAtZero:true}}}});

function showAlert() {
  const popup = document.getElementById('alertPopup');
  popup.style.display='block';
  setTimeout(()=>{popup.style.display='none';},5000);
}

setInterval(()=>{
fetch('/data').then(r=>r.json()).then(d=>{
  document.getElementById('pir').innerHTML = d.pir?"Motion Detected":"No Motion";
  document.getElementById('temp').innerHTML = d.temp+" °C";
  document.getElementById('gas').innerHTML = d.gas;
  const riskCard = document.getElementById('risk'); riskCard.innerHTML=d.fireRisk;
  if(d.fireRisk=="HIGH") { riskCard.style.background="red"; showAlert(); }
  else if(d.fireRisk=="MEDIUM") riskCard.style.background="orange";
  else riskCard.style.background="green";

  const time = new Date().toLocaleTimeString();
  labels.push(time); tempData.push(d.temp); gasData.push(d.gas);
  if(labels.length>20){labels.shift();tempData.shift();gasData.shift();}
  tempChart.update(); gasChart.update();
});
},2000);
</script>
</head>
<body>
<div id="alertPopup">🔥 HIGH FIRE RISK!</div>
<h1>🔥 FireZero AI Dashboard</h1>
<div class="card">PIR Motion: <span id="pir">Loading...</span></div>
<div class="card">Temperature: <span id="temp">Loading...</span></div>
<div class="card">Gas Value: <span id="gas">Loading...</span></div>
<div class="card">Fire Risk: <span id="risk">Loading...</span></div>
<canvas id="tempChart" width="400" height="200"></canvas>
<canvas id="gasChart" width="400" height="200"></canvas>
</body>
</html>
)";
  server.send(200,"text/html",html); //sridevi part finih
}


void handleData(){
  SensorData data = readSensors();
  server.send(200,"application/json",sensorDataToWebJSON(data));
}


void setup(){
  Serial.begin(115200);

  WiFi.begin(ssid,password);
  Serial.print("Connecting to WiFi");
  while(WiFi.status()!=WL_CONNECTED){ delay(500); Serial.print("."); }
  Serial.println("\nConnected! IP: "+WiFi.localIP().toString());

  server.on("/",handleRoot);
  server.on("/data",handleData);
  server.begin();
}


void loop(){
  server.handleClient();
  SensorData data = readSensors();
  sendToWebhook(data); 
  delay(2000); //manish cross check * 1
}
