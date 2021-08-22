const {PiTFT} = require('../');

/**
 * 
 * echo 0 > /sys/class/vtconsole/vtcon1/bind
 */
const display = PiTFT.initDisplay("/dev/fb0");
display.turnBacklightOnOff(true);

function getRandomColor() {
    return Array(3).fill(0).map(() => Math.floor(Math.random() * 255));
}

const colorInterval = setInterval(() => {
    const colorA = getRandomColor();
    const colorB = getRandomColor();
    
    display.drawRect(0, 0, display.width, display.height, colorA, true);
    display.drawRect(30, 30, display.width - 60, display.height - 60, colorB, true);
}, 250);

PiTFT.initButtons((button, state) => {
    console.log("Button", button, "state changed to", state);

    // PRESSED
    if (state === 0) {
        switch (button) {
            case 4: // UP
                display.turnBacklightOnOff(!display.getBacklightOnOffState());
                break;
            case 5: // DOWN
                process.exit(0);
                break;
            default:
                break;
        }
    }
});
