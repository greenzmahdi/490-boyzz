#ifndef HTML_CONTENT_H
#define HTML_CONTENT_H

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>DRO Interface</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: #e7e7e7;
            color: #333;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }

        .dro-container {
            background-color: #d8d8d8;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            margin-bottom: 20px;
        }

        .readout {
            background-color: #000;
            color: #0f0;
            padding: 20px;
            font-size: 1.5em;
            font-family: 'Courier New', Courier, monospace;
            border: 1px solid #333;
            margin-bottom: 10px;
        }



        /* Mode Display */
        #container {
            position: absolute;
            top: 50px;
            left: 50px;
            width: 150px;
            height: 285px;
        }

        /* DRO readout */
        #container1 {
            position: absolute;
            top: 50px;
            left: 250px;
            width: 450px;
        }

        /* Function controls */
        #container2 {
            position: absolute;
            top: 390px;
            left: 393px;
        }

        /* Save coordinate container */
        #container3 {
            position: absolute;
            top: 390px;
            left: 50px;
            width: 293px;
            height: 220px;
        }

        /* Coordinate list*/
        #container12 {
            position: absolute;
            top: 660px;
            left: 50px;
            width: 650px;
            height: 123px;

        }

        /* Zero ut / Selector Buttons */
        #container4 {
            position: absolute;
            top: 50px;
            left: 750px;
            bottom: 490px;
            max-width: 400px;
            height: 130px;
        }

        /* Function buttons */
        #container5 {
            position: absolute;
            top: 810px;
            left: 50px;
        }

        /* Shape helper functions buttons */
        #container6 {
            position: absolute;
            top: 490px;
            left: 393px;
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 5px;
            width: 308px;
            height: 120px;
        }

        /* Arrow Buttons */
        #container7 {
            position: absolute;
            left: 750px;
            top: 645px;
            height: 140px;
            display: flex;
            flex-direction: column;
            align-items: center;
            width: 160px;
            /* Adjust if necessary */
        }

        .arrow-row {
            display: flex;
            justify-content: center;
            align-items: center;
        }

        /* Style adjustments for the first row with the Up arrow */
        .arrow-row.up {
            margin-bottom: 5px;
            /* Space between the up arrow and the middle row */
        }

        .arrow-row.middle {
            width: 100%;
            /* Ensures the middle row takes full width of container */
            justify-content: space-around;
            /* Evenly spaces items in this row */
        }

        .arrow-row button {
            width: 50px;
            /* Width of each button */
            height: 56px;
            /* Height of each button */
            border-radius: 4px;
            background-color: #f5f5f5;
            /* Background color */
            border: 1px solid #ccc;
            /* Border color */
        }

        .arrow-row button:hover {
            background-color: #e0e0e0;
            /* Darker background on hover */
        }

        /* Ensuring that the middle row buttons (left and right) are visually aligned and square */
        .arrow-row.middle button {
            width: 56px;
            /* Making them square */
        }

        /* Adding icons to the buttons (if you use something like Font Awesome) */
        .arrow-row button.up-arrow:before {
            content: '↑';
            /* Up arrow icon */
        }

        .arrow-row button.down-arrow:before {
            content: '↓';
            /* Down arrow icon */
        }

        .arrow-row button.left-arrow:before {
            content: '←';
            /* Left arrow icon */
        }

        .arrow-row button.right-arrow:before {
            content: '→';
            /* Right arrow icon */
        }



        /* Calculator Display*/
        #container11 {
            position: absolute;
            top: 50px;
            left: 965px;
            width: 370px;
        }

        /* Calculator */
        #container8 {
            position: absolute;
            top: 160px;
            left: 965px;
        }

        /* ABS/INC Toggle Buttons, Zero all out etc */
        #container9 {
            position: absolute;
            top: 430px;
            left: 750px;
            width: 160px;

        }

        /* Pulse Factor Adjustment */
        #container10 {
            position: absolute;
            top: 235px;
            left: 750px;
            width: 160px;
            height: 140px;

        }

        /* Coordinate plane display */
        #container13 {
            position: absolute;
            top: 605px;
            left: 970px;
            width: 365px;
            height: 185px;
        }

        .calculator-grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
        }

        .zeroSelect-grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: px;
        }

        button {
            padding: 15px;
            background: #f5f5f5;
            /* Lighter background */
            border: 1px solid #bbb;
            /* Softer border color */
            border-radius: 5px;
            text-align: center;
            cursor: pointer;
            transition: background-color 0.3s;
            /* Smooth transition for hover effect */
        }

        button:hover {
            background: #e0e0e0;
            /* Subtle hover effect */
        }

        /* .wide {
            grid-column: span 3;
        } */

        #calculatorModeBtn {
            /* Add default styles for the button */
            padding: 10px;
            border: none;
            cursor: pointer;
        }

        .calculator-mode-enabled {
            /* Add styles for the button when calculator mode is enabled */
            background-color: green;
            color: white;
        }

        .calculator-mode-disabled {
            /* Add styles for the button when calculator mode is disabled */
            background-color: gray;
            color: white;
        }
    </style>
</head>

<body>


    <!-- Container 1: Readout displays (X, Y, Z) -->

    <div class="dro-container" id="container13">
        <canvas id="coordinateCanvas" width="400" height="400"></canvas>
    </div>


    <div class="dro-container" id="container">
        <div class="readout" id="currentPlane" >1</div>
        <div class="readout" id="modeIndicator">ABS</div>

        <div class="readout modeDisplay" id="mode-readout">
            <span id="modeMeasureIndicator">INCH</span>
        </div>

    </div>

    <div class="dro-container" id="container1">

        <div class="readout" id="x-readout">
            X: <span id="position"></span>
        </div>
        <div class="readout" id="y-readout">
            Y: <span id="position2"></span>
        </div>
        <div class="readout" id="z-readout">
            Z: <span id="position3"></span>
        </div>
        <button onclick="saveCurrentPosition()">
            Save Current Position
        </button>
    </div>

    <!-- Container 2: Function keys -->
    <div class="dro-container" id="container2">
        <button>F1</button>
        <button>F2</button>
        <button>F3</button>
        <button>F4</button>
        <button>F5</button>
        <button>F6</button>
    </div>


    <div class="dro-container" id="container3">
        <div class="dro-container">
            <input type="number" id="xInput" placeholder="X-coordinate" />
            <input type="number" id="yInput" placeholder="Y-coordinate" />
            <input type="number" id="zInput" placeholder="Z-coordinate" />
            <div id="lastPoint">Last Point: None</div>
        </div>
        <button onclick="savePoint()">Save Point</button>
        <button onclick="getAllPoints()">Show All Points</button>
    </div>

    <div class="dro-container" id="container12">

        <ul id="pointsList"></ul>
    </div>


    <div class="dro-container" id="container4">
        <div class="zeroSelect-grid">
            <button onclick="resetPosition('x')">Xo</button>
            <button onclick="resetPosition('select_x')">Select X</button>
            <button onclick="resetPosition('y')">Yo</button>
            <button onclick="resetPosition('select_y')">Select Y</button>
            <button onclick="resetPosition('z')">Zo</button>
            <button onclick="resetPosition('select_z')">Select Z</button>

        </div>
    </div>

    <div class="dro-container" id="container10">
        <form id="factorForm">
            <!-- <label for="factor_mm">Factor (mm per pulse):</label><br /> -->
            <input type="text" id="factor_mm" name="factor_mm" value="Default mm value" /><br />
            <!-- <label for="factor_inch">Factor (inch per pulse):</label><br /> -->
            <input type="text" id="factor_inch" name="factor_inch" value="Default inch value" />
            </br>
            </br>
            <button type="button" onclick="updateFactors()">
                Update Factors
            </button>
        </form>
    </div>

    <div class="dro-container" id="container6">
        <button>Process Holes Circle</button>
        <button>Process Holes Line</button>
        <button>R Cut</button>
        <button>Process Slope</button>
    </div>

    <!-- Container 7: Arrow keys -->
    <div class="dro-container" id="container7">
        <div class="arrow-row up">
            <button class="up-arrow"></button>
        </div>
        <div class="arrow-row middle">
            <button class="left-arrow"></button>
            <button class="down-arrow"></button>
            <button class="right-arrow"></button>
        </div>
    </div>

    <!-- Display the value of the calculator -->
    <div class="dro-container" id="container11">
        <div class="readout">
            <span id="calctemp"></span>
        </div>
    </div>

    <!-- Container 3: Calculator grid -->
    <div class="dro-container" id="container8">
        <div class="calculator-grid">

            <button>1/2</button>
            <button id="toggleButton" onclick="toggleMeasureMode()">
                INCH/MM
            </button>
            <!-- <span id="modeMeasureIndicator">INCH</span> -->
            <!-- <p>Position: <span id="poss">0</span></p> -->

            <button id="calculatorModeBtn" onclick="toggleCalculatorMode()">
                Calculator Mode
            </button>
            <button onclick="addNumber(9)">9</button>
            <button onclick="addNumber(8)">8</button>
            <button onclick="addNumber(7)">7</button>
            <button onclick="addNumber(6)">6</button>
            <button onclick="addNumber(5)">5</button>
            <button onclick="addNumber(4)">4</button>
            <button onclick="addNumber(3)">3</button>
            <button onclick="addNumber(2)">2</button>
            <button onclick="addNumber(1)">1</button>
            <button onclick="addDecimal()">.</button>
            <button onclick="addNumber(0)">0</button>
            <button>+/-</button>

            <button id="plusBtn" class="operator" onclick="addOperation('+')">
                PLUS
            </button>
            <button id="multiplyBtn" class="operator" onclick="addOperation('*')">
                MULTI
            </button>
            <button id="subtractBtn" class="operator" onclick="addOperation('-')">
                SUB
            </button>
            <button id="divideBtn" class="operator" onclick="addOperation('/')">
                DIV
            </button>
            <button onclick="calculate()">EQUAL</button>
        </div>
    </div>

    <!-- Container 4: Additional control buttons -->
    <div class="dro-container" id="container9">
        <div class="zeroSelect-grid">
            <button onclick="toggleMode()">Toggle ABS/INC</button>
            <button id="zeroAllButton" onclick="zeroAllAxis()">XYZo</button>
            <button>CA</button>
            <button>ENT</button>
        </div>
    </div>

    </div>



    <script>
        var currentOperation = null;
        var tempNumber = "";
        var currentValue = 0;
        var calculatorMode = false;

        function updateFactors() {
            var factor_mm = document.getElementById("factor_mm").value;
            var factor_inch = document.getElementById("factor_inch").value;
            fetch("/define-factor", {
                method: "POST",
                headers: {
                    "Content-Type": "application/x-www-form-urlencoded",
                },
                body:
                    "factor_mm=" +
                    encodeURIComponent(factor_mm) +
                    "&factor_inch=" +
                    encodeURIComponent(factor_inch),
            })
                .then((response) => {
                    if (response.ok) {
                        return response.text();
                    } else {
                        throw new Error("Network response was not ok");
                    }
                })
                .then((data) => {
                    document.getElementById("message").innerText = data;
                })
                .catch((error) => {
                    console.error("Error:", error);
                });
        }

        function toggleCalculatorMode() {
            calculatorMode = !calculatorMode;
            updateCalculatorModeButton(); // Update button style
            updateOperatorButtons(); // Update operator buttons based on the calculator mode
        }

        function updateCalculatorModeButton() {
            var calculatorModeBtn = document.getElementById("calculatorModeBtn");
            if (calculatorMode) {
                calculatorModeBtn.classList.add("calculator-mode-enabled");
                calculatorModeBtn.classList.remove("calculator-mode-disabled");
            } else {
                calculatorModeBtn.classList.remove("calculator-mode-enabled");
                calculatorModeBtn.classList.add("calculator-mode-disabled");
            }
        }

        function updateOperatorButtons() {
            var operatorButtons = document.getElementsByClassName("operator");
            for (var i = 0; i < operatorButtons.length; i++) {
                if (calculatorMode) {
                    operatorButtons[i].removeAttribute("disabled");
                } else {
                    operatorButtons[i].setAttribute("disabled", "disabled");
                }
            }
        }

        function addOperation(operation) {
            if (calculatorMode) {
                if (currentOperation === null) {
                    currentOperation = operation;
                    currentValue = parseFloat(
                        document.getElementById("position").innerText
                    );
                    tempNumber = "";
                }
            }
        }

        function addDecimal() {
            // Ensure tempNumber doesn't already contain a decimal point
            if (!tempNumber.includes(".")) {
                tempNumber += ".";
            }
            updateDisplay();
        }

        function addNumber(number) {
            tempNumber += number.toString();
            updateDisplay();
        }

        function calculate() {
            if (currentOperation !== null && tempNumber !== "") {
                switch (currentOperation) {
                    case "+":
                        currentValue += parseFloat(tempNumber);
                        break;
                    case "*":
                        currentValue *= parseFloat(tempNumber);
                        break;
                    case "-":
                        currentValue -= parseFloat(tempNumber);
                        break;
                    case "/":
                        var divisor = parseFloat(tempNumber);
                        if (divisor !== 0) {
                            currentValue /= divisor;
                        } else {
                            alert("Nuclear threat detected");
                        }
                        break;
                }
                currentOperation = null;
                tempNumber = "";
                updateDisplay();
            }
        }

        function updateDisplay() {
            var positionElement = document.getElementById("calctemp");
            positionElement.innerText = currentValue.toString();
        }

        function zeroAllAxis() {
            fetch("/zero-all-axis")
                .then((response) => {
                    if (!response.ok) {
                        throw new Error("Network response was not ok");
                    }
                    return response.text();
                })
                .then((data) => {
                    console.log(data); // Log the server response for debugging.
                    alert(data); // Alert the user or update the status on the page.
                })
                .catch((error) => {
                    console.error(
                        "There was a problem with the fetch operation:",
                        error
                    );
                    alert("Failed to zero all axes: " + error.message); // Provide error feedback.
                });
        }

        function toggleMode() {
            fetch("/toggle-mode")
                .then((response) => response.text())
                .then((mode) => {
                    document.getElementById("modeIndicator").innerText = mode;
                    updatePositions();  // Refresh display to show correct mode values
                })
                .catch(console.error);
        }


        function toggleMeasureMode() {
            fetch("/toggle-measure-mode")
                .then((response) => response.text())
                .then((data) => {
                    // Now using 'modeIndicator' as the ID for the mode display element
                    document.getElementById("modeMeasureIndicator").innerText = data;
                    // updatePosition(); // Update positions if needed, otherwise you can remove this line
                })
                .catch(console.error);
        }



        function updatePositions() {
            fetch("/get-positions")
                .then(response => response.json())
                .then(data => {
                    const mode = document.getElementById("modeIndicator").innerText;
                    document.getElementById("position").innerText = (mode === "ABS" && data.positionX_ABS !== undefined) ? data.positionX_ABS : data.positionX_INC;
                    document.getElementById("position2").innerText = (mode === "ABS" && data.positionY_ABS !== undefined) ? data.positionY_ABS : data.positionY_INC;
                    document.getElementById("position3").innerText = (mode === "ABS" && data.positionZ_ABS !== undefined) ? data.positionZ_ABS : data.positionZ_INC;
                })
                .catch(error => {
                    console.error('Error fetching position data:', error);
                    // Optionally set a default or placeholder text if there is an error
                    document.getElementById("position").innerText = "N/A";
                    document.getElementById("position2").innerText = "N/A";
                    document.getElementById("position3").innerText = "N/A";
                });
        }


        function resetPosition(axis) {
            fetch("/reset/" + axis)
                .then((response) => {
                    if (response.ok) {
                        console.log(axis.toUpperCase() + " position reset"); // print message
                        // updatePositions(); // Refresh the positions immediately
                    }
                })
                .catch(console.error);
        }

        function updatePlaneDisplay() {
            fetch("/get-current-plane")
                .then((response) => response.text())
                .then((data) => {
                    console.log("Updated Plane Index: ", data); // Log for debugging
                    document.getElementById("currentPlane").innerText =
                        "Plane: " + data;
                        updatePositions();
                })
                .catch((error) =>
                    console.error("Failed to update plane display:", error)
                );
        }

        function updatePositionsAndPlane() {
            updatePositions();
            updatePlaneDisplay();
        }

        function savePoint() {
            const x = document.getElementById("xInput").value;
            const y = document.getElementById("yInput").value;
            const z = document.getElementById("zInput").value;

            fetch(`/add-point?x=${x}&y=${y}&z=${z}`, { method: "POST" })
                .then((response) => response.text())
                .then((data) => {
                    alert("Point saved");
                    getLastPoint(); // Fetch the last point after saving
                })
                .catch((error) => console.error("Error saving point:", error));
        }

        function getLastPoint() {
            fetch("/get-last-point")
                .then((response) => response.text())
                .then((data) => {
                    document.getElementById("lastPoint").innerText =
                        "Last Point: " + data;
                })
                .catch((error) => {
                    console.error("Error fetching last point:", error);
                    document.getElementById("lastPoint").innerText =
                        "Last Point: Error";
                });
        }

        function getAllPoints() {
            fetch("/get-all-points")
                .then((response) => {
                    if (!response.ok) throw new Error("No points found");
                    return response.json();
                })
                .then((data) => {
                    const pointsList = document.getElementById("pointsList");
                    pointsList.innerHTML = ""; // Clear existing points

                    data.points.forEach((point, index) => {
                        const pointItem = document.createElement("button");
                        pointItem.textContent = `Point ${index + 1}: (${point.x}, ${point.y
                            }, ${point.z})`;
                        pointItem.addEventListener("click", () => {
                            displayPoint(point);
                        });
                        pointsList.appendChild(pointItem);
                    });
                })
                .catch((error) => {
                    console.error("Error fetching all points:", error);
                    document.getElementById("pointsList").innerText =
                        "Failed to load points";
                });
        }

        function displayPoint(point) {
            document.getElementById("position").textContent = point.x;
            document.getElementById("position2").textContent = point.y;
            document.getElementById("position3").textContent = point.z;
        }

        function saveCurrentPosition() {
            fetch("/save-current-position")
                .then((response) => {
                    if (response.ok) {
                        return response.text();
                    }
                    throw new Error("Could not save position");
                })
                .then((message) => {
                    console.log(message);
                    getAllPoints(); // Refresh the points display
                })
                .catch((error) => console.error("Error:", error));

            const canvas = document.getElementById('coordinateCanvas');

            const ctx = canvas.getContext('2d');

            function drawGrid() {
                const width = canvas.width;
                const height = canvas.height;
                ctx.beginPath();
                ctx.strokeStyle = '#ccc';

                // Draw grid lines every 50 pixels
                for (let x = 0; x <= width; x += 50) {
                    ctx.moveTo(x, 0);
                    ctx.lineTo(x, height);
                }
                for (let y = 0; y <= height; y += 50) {
                    ctx.moveTo(0, y);
                    ctx.lineTo(width, y);
                }
                ctx.stroke();
            }

            function drawPoints(points) {
                ctx.fillStyle = 'red';
                points.forEach(point => {
                    ctx.beginPath();
                    ctx.arc(point.x, point.y, 5, 0, Math.PI * 2, true); // Draw circle for each point
                    ctx.fill();
                });
            }

            drawGrid();

        }
        setInterval(updatePositionsAndPlane, 1000); // Adjust interval to 1000 ms
        setInterval(updatePositions, 50); // Call updatePositions() every 1000ms (1 second) but right now it is 50ms so stupid fast
    </script>
</body>

</html>
)rawliteral";

#endif