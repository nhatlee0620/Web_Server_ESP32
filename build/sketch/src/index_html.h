#line 1 "/home/nhatlee/Desktop/Test/web_server_DATN/src/index_html.h"
const char PROGMEM INDEX2_HTML[] = R"=====(
<!DOCTYPE html>
<html>

<head>
  <title>ESP Wi-Fi Manager</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
      font-family: Arial, Helvetica, sans-serif;
      display: inline-block;
      text-align: center;
    }

    h1 {
      font-size: 1.8rem;
      color: white;
    }

    h2 {
      font-size: 1.8rem;
      color: rgb(236, 24, 24);
    }

    p {
      font-size: 1.3rem;
    }

    .topnav {
      overflow: hidden;
      background-color: #0A1128;
    }

    body {
      margin: 0;
    }

    .content {
      padding: 5%;
    }

    .card-grid {
      max-width: 800px;
      margin: 0 auto;
      display: grid;
      grid-gap: 2rem;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
    }

    .card {
      background-color: white;
      box-shadow: 2px 2px 12px 1px rgba(140, 140, 140, .5);
    }

    .card-title {
      font-size: 1.2rem;
      font-weight: bold;
      color: #034078
    }

    input[type=submit] {
      border: none;
      color: #FEFCFB;
      background-color: #034078;
      padding: 15px 15px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 16px;
      width: 100px;
      margin-right: 10px;
      border-radius: 4px;
      transition-duration: 0.4s;
    }

    input[type=submit]:hover {
      background-color: #1282A2;
    }

    input[type=text],
    input[type=number],
    select {
      width: 50%;
      padding: 12px 20px;
      margin: 18px;
      display: inline-block;
      border: 1px solid #ccc;
      border-radius: 4px;
      box-sizing: border-box;
    }

    label {
      font-size: 1.2rem;
    }

    .value {
      font-size: 1.2rem;
      color: #1282A2;
    }

    .state {
      font-size: 1.2rem;
      color: #1282A2;
    }

    .switch {
      position: relative;
      display: inline-block;
      width: 60px;
      height: 34px;
    }

    .switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }

    .slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #ccc;
      -webkit-transition: .4s;
      transition: .4s;
    }

    .slider:before {
      position: absolute;
      content: "";
      height: 26px;
      width: 26px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      -webkit-transition: .4s;
      transition: .4s;
    }

    input:checked+.slider {
      background-color: #2196F3;
    }

    input:focus+.slider {
      box-shadow: 0 0 1px #2196F3;
    }

    input:checked+.slider:before {
      -webkit-transform: translateX(26px);
      -ms-transform: translateX(26px);
      transform: translateX(26px);
    }

    /* Rounded sliders */
    .slider.round {
      border-radius: 34px;
    }

    .slider.round:before {
      border-radius: 50%;
    }
  </style>
</head>

<body>
  <div class="topnav">
    <h1>SUPPORT PICKING WIFI CONFIG</h1>
  </div>
  <div class="content">
    <div class="card-grid">
      <div class="card">
        <form action="/control" accept-charset="utf-8">
          <h2>CONNECT TO WIFI</h2>
          <p>
            <label for="ssid">SSID</label>
            <input type="text" id="ssid" name="ssid"><br>
            <label for="pass">Password</label>
            <input type="text" id="pass" name="pass"><br>
            <input type="submit" value="Submit">
          </p>
        </form>
      </div>
    </div>
    <h2>LIGHT FLOWER MODE</h2>
    <label class="switch">
      <input type="checkbox" id="output" onchange="toggleCheckbox(this)">
      <span class="slider round"></span>
    </label>
  </div>
  <p id="autoMode"><strong></strong></p>
  <script>
    function toggleCheckbox(element) {
      var xhr = new XMLHttpRequest();
      if (element.checked) { xhr.open("GET", "/control?auto=true", true); }
      else { xhr.open("GET", "/control?auto=false", true); }
      xhr.send();
    }

    setInterval(function () {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          var inputChecked;
          var outputStateM;
          if (this.responseText == 1) {
            inputChecked = true;
            outputStateM = "Mode: Auto";
          }
          else {
            inputChecked = false;
            outputStateM = "Mode: Semi Auto";
          }
          document.getElementById("output").checked = inputChecked;
          document.getElementById("autoMode").innerHTML = outputStateM;
        }
      };
      xhttp.open("GET", "/getState", true);
      xhttp.send();
    }, 5000);
  </script>
</body>

</html>
)=====";