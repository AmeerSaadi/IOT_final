#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DIYables_4Digit7Segment_74HC595.h>

#define CLOCK_PIN D5  
#define LATCH_PIN D6 
#define DATA_PIN D7  

#define LOCK_PIN D4  

DIYables_4Digit7Segment_74HC595 display(CLOCK_PIN, LATCH_PIN, DATA_PIN);

IPAddress localIP(192, 168, 1, 1);

const char* networkSSID = "EscapeRoom";
const char* networkPassword = "12345678";

String secretCode = "";
unsigned long lastWifiCheck = 0;

ESP8266WebServer server(80);

void handleUnknownRequest() {
  String response = "Requested resource not found\n\n";
  response += "URI: ";
  response += server.uri();
  response += "\nMethod: ";
  response += (server.method() == HTTP_GET) ? "GET" : "POST";
  response += "\nArguments: ";
  response += server.args();
  response += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    response += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", response);
}

void handleCodeSubmission() {
  Serial.println("Code submission received");
  if(server.hasArg("unlock")){
    digitalWrite(LOCK_PIN, LOW);
  }
  if (server.hasArg("code") && secretCode.length() < 4) {
    if (server.arg("code").length() == 1) {
      secretCode = secretCode + server.arg("code");
      display.printInt(secretCode.toInt(), true);
      server.send(200, "text/plain", "Code accepted");
    }
    server.send(400, "text/plain", "Code must be exactly 1 character");
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
}

void WiFisetup() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(localIP, localIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(networkSSID, networkPassword);

  server.on("/", handleHomePage);
  server.on("/submit", handleCodeSubmission);
  server.onNotFound(handleUnknownRequest);

  Serial.print("Access Point IP: ");
  Serial.println(localIP);
  pinMode(LOCK_PIN, OUTPUT);
  lastWifiCheck = millis();
  display.clear();
  display.printInt(0, true);
  server.begin();
}

void WiFiloop() {
  if (millis() - lastWifiCheck >= 10) {
    lastWifiCheck = millis();
    server.handleClient();
  }
  display.loop();
}

void handleHomePage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="he">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Escape Room</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700&display=swap');

        :root {
            --primary: #ff6f61;
            --primary-dark: #cc564b;
            --accent: #00bcd4;
            --error: #ff1744;
            --success: #4caf50;
            --dark: #1a1a1a;
            --light-text: #ffffff;
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
            background-color: #000;
            font-family: 'Orbitron', sans-serif;
            overflow: hidden;
            position: relative;
        }

        .background-container {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            z-index: -1;
            overflow: hidden;
        }

        .background-overlay {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.7);
            z-index: 1;
        }

        .background-image {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            filter: blur(5px);
            transform: scale(1.1);
            z-index: 0;
            background-color: #0d1117;
        }

        .cyberpunk-grid {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background-image: linear-gradient(rgba(18, 16, 16, 0) 2px, transparent 2px),
                              linear-gradient(90deg, rgba(18, 16, 16, 0) 2px, transparent 2px);
            background-size: 50px 50px;
            background-position: -2px -2px;
            z-index: 1;
            opacity: 0.6;
        }

        .container {
            position: relative;
            width: 90%;
            max-width: 500px;
            z-index: 10;
        }

        .game-console {
            background: rgba(20, 20, 20, 0.9);
            backdrop-filter: blur(20px);
            border-radius: 15px;
            border: 1px solid rgba(255, 111, 97, 0.4);
            box-shadow: 0 25px 60px rgba(0, 0, 0, 0.9),
                        0 0 0 1px rgba(255, 255, 255, 0.1),
                        inset 0 0 40px rgba(255, 111, 97, 0.3);
            padding: 3rem;
            position: relative;
            overflow: hidden;
        }

        .game-console::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 6px;
            background: linear-gradient(90deg, var(--primary), var(--accent));
            z-index: 2;
        }

        .glowing-edges {
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            border: 1px solid rgba(255, 111, 97, 0.3);
            border-radius: 15px;
            box-shadow: 0 0 20px rgba(255, 111, 97, 0.4);
            pointer-events: none;
            z-index: -1;
        }

        .header {
            text-align: center;
            margin-bottom: 2.5rem;
            position: relative;
        }

        h2 {
            color: var(--light-text);
            font-size: 2rem;
            font-weight: 700;
            text-shadow: 0 0 15px rgba(255, 111, 97, 0.8);
            margin-bottom: 0.75rem;
            letter-spacing: 2px;
        }

        .subtitle {
            color: rgba(255, 255, 255, 0.8);
            font-size: 1rem;
            font-weight: 400;
            margin-top: 0.75rem;
        }

        .lock-icon {
            font-size: 3rem;
            color: var(--primary);
            margin-bottom: 1.5rem;
            animation: glow 2s infinite alternate;
        }

        @keyframes glow {
            from {
                text-shadow: 0 0 10px rgba(255, 111, 97, 0.6),
                            0 0 20px rgba(255, 111, 97, 0.6);
            }
            to {
                text-shadow: 0 0 20px rgba(255, 111, 97, 0.9),
                            0 0 40px rgba(255, 111, 97, 0.9),
                            0 0 60px rgba(255, 111, 97, 0.6);
            }
        }

        .code-form {
            position: relative;
            margin: 2.5rem 0;
        }

        .input-group {
            position: relative;
            margin-bottom: 2rem;
        }

        input[type='text'] {
            width: 100%;
            background-color: rgba(255, 255, 255, 0.1);
            color: var(--light-text);
            border: 2px solid rgba(255, 111, 97, 0.4);
            border-radius: 10px;
            font-size: 1.5rem;
            letter-spacing: 4px;
            text-align: center;
            padding: 18px 24px;
            transition: all 0.3s ease;
            box-shadow: 0 8px 20px rgba(0, 0, 0, 0.3),
                        inset 0 3px 8px rgba(0, 0, 0, 0.3);
        }

        input[type='text']:focus {
            outline: none;
            border-color: var(--primary);
            box-shadow: 0 0 0 4px rgba(255, 111, 97, 0.3),
                        0 8px 20px rgba(0, 0, 0, 0.3),
                        inset 0 3px 8px rgba(0, 0, 0, 0.3);
        }

        input[type='text']::placeholder {
            color: rgba(255, 255, 255, 0.4);
        }

        .input-decoration {
            position: absolute;
            top: -12px;
            right: -12px;
            width: 24px;
            height: 24px;
            border-top: 3px solid var(--primary);
            border-right: 3px solid var(--primary);
            opacity: 0.8;
        }

        button {
            width: 100%;
            background: linear-gradient(45deg, var(--primary), var(--primary-dark));
            color: white;
            border: none;
            border-radius: 10px;
            padding: 18px 24px;
            font-size: 1.25rem;
            font-weight: 600;
            letter-spacing: 2px;
            cursor: pointer;
            transition: all 0.3s ease;
            margin-top: 1.5rem;
            box-shadow: 0 8px 20px rgba(0, 0, 0, 0.4),
                        0 0 0 1px rgba(255, 255, 255, 0.1);
            position: relative;
            overflow: hidden;
            text-transform: uppercase;
        }

        button:hover {
            background: linear-gradient(45deg, var(--primary-dark), var(--primary));
            transform: translateY(-4px);
            box-shadow: 0 10px 25px rgba(0, 0, 0, 0.5),
                        0 0 0 1px rgba(255, 255, 255, 0.15);
        }

        button:active {
            transform: translateY(2px);
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.4),
                        0 0 0 1px rgba(255, 255, 255, 0.1);
        }

        button::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: linear-gradient(
                90deg,
                transparent,
                rgba(255, 255, 255, 0.3),
                transparent
            );
            transition: all 0.6s;
        }

        button:hover::before {
            left: 100%;
        }

        .button-icon {
            margin-left: 12px;
        }

        .feedback {
            margin-top: 2rem;
            padding: 1.25rem;
            border-radius: 10px;
            font-weight: 600;
            text-align: center;
            display: flex;
            align-items: center;
            justify-content: center;
            box-shadow: 0 8px 20px rgba(0, 0, 0, 0.3);
        }

        .fail {
            background-color: rgba(255, 23, 68, 0.2);
            color: #ff6b6b;
            border: 1px solid rgba(255, 23, 68, 0.4);
            animation: shake 0.5s cubic-bezier(.36,.07,.19,.97) both;
        }

        @keyframes shake {
            0%, 100% { transform: translateX(0); }
            10%, 30%, 50%, 70%, 90% { transform: translateX(-8px); }
            20%, 40%, 60%, 80% { transform: translateX(8px); }
        }

        .currect {
            background-color: rgba(76, 175, 80, 0.2);
            color: #69f0ae;
            border: 1px solid rgba(76, 175, 80, 0.4);
            animation: success-pulse 2s infinite;
            font-size: 1.8rem;
            padding: 2rem;
        }

        @keyframes success-pulse {
            0% { box-shadow: 0 0 0 0 rgba(76, 175, 80, 0.5); }
            70% { box-shadow: 0 0 0 20px rgba(76, 175, 80, 0); }
            100% { box-shadow: 0 0 0 0 rgba(76, 175, 80, 0); }
        }

        .feedback-icon {
            margin-left: 12px;
            font-size: 1.4rem;
        }

        .scan-line {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 10px;
            background: linear-gradient(
                to bottom,
                rgba(255, 255, 255, 0.2),
                transparent
            );
            animation: scan 6s linear infinite;
            opacity: 0.6;
            pointer-events: none;
        }

        @keyframes scan {
            0% { top: 0; }
            100% { top: 100%; }
        }

        .status-lights {
            position: absolute;
            top: 20px;
            right: 20px;
            display: flex;
            gap: 10px;
        }

        .status-light {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            animation: blink 3s infinite;
        }

        .status-light:nth-child(1) {
            background-color: var(--primary);
            animation-delay: 0s;
        }

        .status-light:nth-child(2) {
            background-color: var(--accent);
            animation-delay: 1s;
        }

        .status-light:nth-child(3) {
            background-color: var(--error);
            animation-delay: 2s;
        }

        @keyframes blink {
            0%, 50%, 100% { opacity: 1; }
            25%, 75% { opacity: 0.6; }
        }

        @media (max-width: 768px) {
            .container {
                width: 95%;
            }
            
            h2 {
                font-size: 1.75rem;
            }
            
            .subtitle {
                font-size: 0.9rem;
            }
            
            input[type='text'] {
                font-size: 1.3rem;
            }
            
            button {
                font-size: 1.1rem;
            }
        }
    </style>
</head>
<body dir="rtl">
    <div class="background-container">
        <div class="background-image"></div>
        <div class="background-overlay"></div>
        <div class="cyberpunk-grid"></div>
    </div>
    
    <div class="container">
        <div class="game-console">
            <div class="glowing-edges"></div>
            <div class="status-lights">
                <div class="status-light"></div>
                <div class="status-light"></div>
                <div class="status-light"></div>
            </div>
            
            <div class="scan-line"></div>
            <div class="header">
                <h2>Security System</h2>
                <p class="subtitle">Enter the secret code to unlock the door</p>
            </div>
            
            <form method="get" class="code-form">
                <div class="input-group">
                    <div class="input-decoration"></div>
                    <input type="text" name="code" placeholder="* * * *" maxlength="4" autocomplete="off">
                </div>
                <button type="submit">
                    Unlock
                </button>
            </form>
        </div>
    </div>
    
    <script>
        document.addEventListener('DOMContentLoaded', function() {
            const code
                        const codeInput = document.querySelector('input[name="code"]');
            
            if (codeInput) {
                codeInput.addEventListener('focus', function() {
                    document.querySelector('.game-console').style.boxShadow = '0 20px 50px rgba(0, 0, 0, 0.8), 0 0 0 1px rgba(255, 255, 255, 0.1), inset 0 0 30px rgba(255, 111, 97, 0.3)';
                });
                
                codeInput.addEventListener('blur', function() {
                    document.querySelector('.game-console').style.boxShadow = '0 20px 50px rgba(0, 0, 0, 0.8), 0 0 0 1px rgba(255, 255, 255, 0.1), inset 0 0 30px rgba(255, 111, 97, 0.2)';
                });
            }
        });
    </script>
</body>
</html>)rawliteral";

server.send(200, "text/html", html);
}
