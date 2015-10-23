
var gadgets = [
  {'KEY_GADGET_NAME' : 'CO2', 'KEY_GADGET_NUMBER' : 0},
  {'KEY_GADGET_NAME' : 'Temperature', 'KEY_GADGET_NUMBER' : 1},
  {'KEY_GADGET_NAME' : 'Humidity', 'KEY_GADGET_NUMBER' : 2},
];

var figures = [
  {'KEY_GADGET_VALUE' : 589, 'KEY_GADGET_TYPE' : 0, 'KEY_GADGET_STRING' : '589'},
  {'KEY_GADGET_VALUE' : (25.3 * 10), 'KEY_GADGET_TYPE' : 1, 'KEY_GADGET_STRING' : '25.3'},
  {'KEY_GADGET_VALUE' : (45.42 * 20), 'KEY_GADGET_TYPE' : 2, 'KEY_GADGET_STRING' : '45.42'},
];

var sentSuccessfully = function(e) {
  console.log('Shedule sent to Pebble successfully!');
};

var sentUnsuccessfully = function(e) {
  console.log('Error sending shedule to Pebble!');
};

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log ("reseived: " + e.payload.KEY_GADGET_COMMAND);
    switch (e.payload.KEY_GADGET_COMMAND) {

      case "send_next_gadget":
        console.log ("send_next_gadget ");
        console.log ("gadgets.length " + gadgets.length);
        if (gadgets.length > 0) {
          console.log ("send");
          Pebble.sendAppMessage(gadgets.shift(), sentSuccessfully, sentUnsuccessfully);
          console.log ("sent");
        } else {
          Pebble.sendAppMessage({
            'KEY_FIGURES_SENT': 1,
          }, sentSuccessfully, sentUnsuccessfully);
        }
       break;

      case "send_figures":
        if (figures.length > 0) {
          console.log ("send figures count" + figures.length);
          Pebble.sendAppMessage({
            'KEY_FIGURE_COUNT': figures.length
          }, sentSuccessfully, sentUnsuccessfully);
        } else {
          Pebble.sendAppMessage({
            'KEY_FIGURES_SENT': 2,
          }, sentSuccessfully, sentUnsuccessfully);
        }
       break;

      case "send_next_figure":
        console.log ("send_next_figure ");
        console.log ("figures.length " + figures.length);
        if (figures.length > 0) {
          console.log ("send");
          Pebble.sendAppMessage(figures.shift(), sentSuccessfully, sentUnsuccessfully);
          console.log ("sent");
        } else {
          Pebble.sendAppMessage({
            'KEY_FIGURES_SENT': 2,
          }, sentSuccessfully, sentUnsuccessfully);
        }
       break;
    }
  }                     
);

function sendData() {
  Pebble.sendAppMessage({
    'KEY_GADGET_COUNT': gadgets.length
  }, sentSuccessfully, sentUnsuccessfully);
}


// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');

    sendData();
  }
);


