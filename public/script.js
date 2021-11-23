const humidity = document.querySelector("#humidity");
const temperature = document.querySelector("#temperature");
const heatIndex = document.querySelector("#heatIndex");
const soilMoisture = document.querySelector("#soilMoisture");
const soilMoistureThreshold = document.querySelector("#soilMoistureThreshold");
const isAutoWater = document.querySelector("#isAutoWater");
const isWater = document.querySelector("#isWater");
const isCover = document.querySelector("#isCover");

const thresholdSub = document.querySelector("#thresholdSub");
const thresholdAdd = document.querySelector("#thresholdAdd");
const autoWater = document.querySelector("#autoWater");
const water = document.querySelector("#water");
const cover = document.querySelector("#cover");

let sensorData;
var HOST = location.origin.replace(/^http/, 'ws')
const socket = new WebSocket(HOST);
socket.addEventListener("open", () => {
  console.log("Connected To Server");
});

socket.onmessage = (message) => {
  sensorData = JSON.parse(message.data);
  console.log(sensorData);
  humidity.innerHTML = sensorData.humidity;
  temperature.innerHTML = sensorData.temperature;
  heatIndex.innerHTML = Math.round((sensorData.heatIndex + Number.EPSILON) * 100) / 100;
  soilMoisture.innerHTML = sensorData.soilMoisture;
  soilMoistureThreshold.innerHTML = sensorData.soilMoistureThreshold;
  isAutoWater.innerHTML = (sensorData.isAutoWater) ? "Yes" : "No";
  isWater.innerHTML = (sensorData.isWater) ? "Yes" : "No";
  isCover.innerHTML = (sensorData.isCover) ? "Yes" : "No";

  console.log(isAutoWater, sensorData.isAutoWater);
  console.log(isWater, sensorData.isWater);
  console.log(isCover, sensorData.isCover);

  autoWater.checked = sensorData.isAutoWater;
  water.checked = sensorData.isWater;
  cover.checked = sensorData.isCover;
};

autoWater.addEventListener("click", () => {
  const data = {
    soilMoistureThreshold: sensorData.soilMoistureThreshold,
    isAutoWater: (autoWater.checked) ? true : false,
    isWater: sensorData.isWater,
    isCover: sensorData.isCover
  }
  socket.send(JSON.stringify(data));
})

water.addEventListener("click", () => {
  const data = {
    soilMoistureThreshold: sensorData.soilMoistureThreshold,
    isAutoWater: sensorData.isAutoWater,
    isWater: true,
    isCover: sensorData.isCover
  }
  socket.send(JSON.stringify(data));
})

cover.addEventListener("click", () => {
  const data = {
    soilMoistureThreshold: sensorData.soilMoistureThreshold,
    isAutoWater: sensorData.isAutoWater,
    isWater: sensorData.isWater,
    isCover: (cover.checked) ? true : false,
  }
  socket.send(JSON.stringify(data));
})

thresholdAdd.addEventListener("click", () => {
  const data = {
    soilMoistureThreshold: sensorData.soilMoistureThreshold + 1,
    isAutoWater: sensorData.isAutoWater,
    isWater: sensorData.isWater,
    isCover: sensorData.isCover
  }
  socket.send(JSON.stringify(data));
})

thresholdSub.addEventListener("click", () => {
  const data = {
    soilMoistureThreshold: sensorData.soilMoistureThreshold - 1,
    isAutoWater: sensorData.isAutoWater,
    isWater: sensorData.isWater,
    isCover: sensorData.isCover
  }
  socket.send(JSON.stringify(data));
})