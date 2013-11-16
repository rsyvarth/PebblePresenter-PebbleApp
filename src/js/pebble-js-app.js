// Fetch saved symbol from local storage (using standard localStorage webAPI)
// var symbol = localStorage.getItem("symbol");

// // We use the fake "PBL" symbol as default
// if (!symbol) {
//   symbol = "PBL";
// }

function fetchAuthKey() {
  var response;
  var req = new XMLHttpRequest();
  console.log('Getting pres info');
  // build the GET request
  var info = localStorage.getItem("auth_key") ? localStorage.getItem("auth_key") : '';
  req.open('GET', "http://pebblepresenter.syvarth.com/getPresentationPebble/"+info, true);
  req.onload = function(e) {
  	console.log(req.readyState);
	if (req.readyState == 4) {
	  // 200 - HTTP OK
	  if(req.status == 200) {
		console.log(req.responseText);
		response = JSON.parse(req.responseText);
		if (response.auth_key) {
		    Pebble.sendAppMessage({symbol: response.auth_key.toString()});
		    localStorage.setItem("auth_key", response.auth_key);
		    localStorage.setItem("pebble_id", response.pebble_id);
		} else if( response.pres_id ) {
			Pebble.sendAppMessage({symbol: 10});//Number of slides
		}
	  } else {
		console.log("Request returned error code " + req.status.toString());
	  }
	}
  }
  req.send(null);
}

function changeSlide(direction) {
  var response;
  var req = new XMLHttpRequest();
  console.log('Change the slide');
  console.log(direction);
  var direct = (direction == -1) ? 'next' : 'back';
  // build the GET request
  req.open('GET', "http://pebblepresenter.syvarth.com/changeSlide/"+localStorage.getItem("pebble_id")+"/"+direct, true);
  req.onload = function(e) {
	if (req.readyState == 4) {
	  // 200 - HTTP OK
	  if(req.status == 200) {
		console.log(req.responseText);
		Pebble.sendAppMessage({price: 0});
	  } else {
		console.log("Request returned error code " + req.status.toString());
	  }
	}
  }
  req.send(null);
}

// Set callback for the app ready event
Pebble.addEventListener("ready",
						function(e) {
						  console.log("connect!" + e.ready);
						  console.log(e.type);
						});

// Set callback for appmessage events
Pebble.addEventListener("appmessage",
						function(e) {
						  console.log("message caught");
						  console.log(e.payload.price);
						  console.log(e.payload.symbol);
						  

						  if( e.payload.price == 1 || e.payload.price == -1 ) { // GetAuthKey / getSlides
						  	  changeSlide(e.payload.price);
						  }

						   if( e.payload.symbol == 'refr' ) { // changeSlide
						  	  fetchAuthKey();
						   }

						});

