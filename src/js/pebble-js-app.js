var maxTriesForSendingAppMessage = 3;
var timeoutForAppMessageRetry = 3000;
var timeoutForRequest = 20000;

function sendAppMessage(message, numTries, transactionId) {
	numTries = numTries || 0;
	if (numTries < maxTriesForSendingAppMessage) {
		numTries++;
		console.log('Sending AppMessage to Pebble: ' + JSON.stringify(message));
		Pebble.sendAppMessage(
			message, function() {}, function(e) {
				console.log('Failed sending AppMessage for transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
				setTimeout(function() {
					sendAppMessage(message, numTries, e.data.transactionId);
				}, 3000);
			}
		);
	} else {
		console.log('Failed sending AppMessage for transactionId:' + transactionId + '. Bailing. ' + JSON.stringify(message));
	}
}

function fetchAuthKey( pebble_id, cb ) {
	var xhr = new XMLHttpRequest();

	var pebble_key = ( pebble_id && localStorage.getItem("pebble_key") ) ? localStorage.getItem("pebble_key") : '';

	sendAppMessage({
		'auth': pebble_key ? pebble_key : '----'
	});

	if( !pebble_key || !pebble_id ) {
		localStorage.setItem("auth_key", '');
 	    localStorage.setItem("pebble_id", '');
 	    pebble_id = '';
	}

	console.log('Getting pres info for: ' + pebble_id);

	xhr.open('GET', 'http://pebblepresenter.syvarth.com/getPresentationPebble/' + pebble_id, true);
	xhr.timeout = timeoutForRequest;
	xhr.onload = function(e) {
		if (xhr.readyState == 4) {
			if (xhr.status == 200) {
				console.log( xhr.responseText );
				res = JSON.parse(xhr.responseText);

				if( res.pres_id ) {
					sendAppMessage({
						'auth': res.auth_key
					});
				} else if( auth_key ) {
					sendAppMessage({
						'auth': res.auth_key
					});

					localStorage.setItem("auth_key", response.auth_key);
 	            	localStorage.setItem("pebble_id", response.pebble_id);
				}

				if( typeof cb == 'function' ) {
					cb();
				}
			} else {
				console.log('Request returned error code ' + xhr.status.toString());
				sendAppMessage({'title': 'Error: ' + xhr.statusText.substring(0,22)});
			}
		}
	}
	xhr.ontimeout = function() {
		sendAppMessage({'title': 'Error: Request timed out!'});
	};
	xhr.onerror = function() {
		sendAppMessage({'title': 'Error: Failed to connect!'});
	};
	xhr.send(null);
}

function changeSlide(direction) {
	var pebble_id = localStorage.getItem("pebble_id");
	if( pebble_id ) {
		_changeSlide(direction);
	} else {
		fetchAuthKey(0, function(){
			_changeSlide(direction);
		});
	}
}

function _changeSlide(direction) {
	var xhr = new XMLHttpRequest();

	console.log('Change the slide: ' + direction);

	var direct = (direction == 'next') ? 'next' : 'back';

	xhr.open('GET', "http://pebblepresenter.syvarth.com/changeSlide/"+localStorage.getItem("pebble_id")+"/"+direct, true);
	xhr.timeout = timeoutForRequest;
	xhr.onload = function(e) {
		if (xhr.readyState == 4) {
			if (xhr.status == 200) {
				console.log( xhr.responseText );
				res = JSON.parse(xhr.responseText);

			} else {
				console.log('Request returned error code ' + xhr.status.toString());
				sendAppMessage({'title': 'Error: ' + xhr.statusText.substring(0,22)});
			}
		}
	}
	xhr.ontimeout = function() {
		sendAppMessage({'title': 'Error: Request timed out!'});
	};
	xhr.onerror = function() {
		sendAppMessage({'title': 'Error: Failed to connect!'});
	};
	xhr.send(null);

  // build the GET request
  // req.open('GET', "http://pebblepresenter.syvarth.com/changeSlide/"+localStorage.getItem("pebble_id")+"/"+direct, true);
  // req.onload = function(e) {
  //       if (req.readyState == 4) {
  //         // 200 - HTTP OK
  //         if(req.status == 200) {
  //               console.log(req.responseText);
  //               Pebble.sendAppMessage({price: 0});
  //         } else {
  //               console.log("Request returned error code " + req.status.toString());
  //         }
  //       }
  // }
  // req.send(null);
}

// function makeRequestToVLC(server_host, server_pass, request) {
	// 	var xhr = new XMLHttpRequest();
	// 	xhr.open('GET', 'http://' + server_host + '/requests/status.json?' + request, true, '', server_pass);
	// 	xhr.timeout = timeoutForVLCRequest;
	// 	xhr.onload = function(e) {
	// 		if (xhr.readyState == 4) {
	// 			if (xhr.status == 200) {
	// 				res    = JSON.parse(xhr.responseText);
	// 				title  = res.information.category.meta.filename || 'VLC Remote';
	// 				title  = title.substring(0,30);
	// 				status = res.state ? res.state.charAt(0).toUpperCase()+res.state.slice(1) : 'Unknown';
	// 				status = status.substring(0,30);
	// 				volume = res.volume || 0;
	// 				volume = (volume / 512) * 200;
	// 				volume = (volume > 200) ? 200 : volume;
	// 				volume = Math.round(volume).toString() + '%';
	// 				sendAppMessage({
	// 					'title': title,
	// 					'status': status,
	// 					'volume': volume
	// 				});
	// 			} else {
	// 				console.log('Request returned error code ' + xhr.status.toString());
	// 				sendAppMessage({'title': 'Error: ' + xhr.statusText});
	// 			}
	// 		}
	// 	}
	// 	xhr.ontimeout = function() {
	// 		sendAppMessage({'title': 'Error: Request timed out!'});
	// 	};
	// 	xhr.onerror = function() {
	// 		sendAppMessage({'title': 'Error: Failed to connect!'});
	// 	};
	// 	xhr.send(null);
// }

Pebble.addEventListener('ready', function(e) {});

Pebble.addEventListener('appmessage', function(e) {
	console.log('AppMessage received from Pebble: ' + JSON.stringify(e.payload));

	// var server_host = localStorage.getItem('server_host');
	// var server_pass = localStorage.getItem('server_pass');

	// if (!e.payload.request) {
	// 	console.log('server_host, server_pass, or request not set');
	// 	return;
	// }

	switch (e.payload.request) {
		// case 'play_pause':
		// 	makeRequestToVLC(server_host, server_pass, 'command=pl_pause');
		// 	break;
		case 'next':
			changeSlide('next');
			break;
		case 'prev':
			changeSlide('prev');
			break;
		// case 'vol_min':
		// 	makeRequestToVLC(server_host, server_pass, 'command=volume&val=0');
		// 	break;
		// case 'vol_max':
		// 	makeRequestToVLC(server_host, server_pass, 'command=volume&val=512');
		// 	break;
		case 'refresh':
			var pebble_id = localStorage.getItem("pebble_id") ? localStorage.getItem("pebble_id") : '';

			fetchAuthKey( pebble_id );
			//makeRequestToVLC(server_host, server_pass, 'command=volume&val=512');
			break;
		//case 'refresh':
		default:
			console.log('Command not recognized');
			//makeRequestToVLC(server_host, server_pass, 'refresh');
			break;
	}
});

// Pebble.addEventListener('showConfiguration', function(e) {
// 	var server_host = localStorage.getItem('server_host') || '';
// 	var server_pass = localStorage.getItem('server_pass') || '';
// 	var uri = 'https://rawgithub.com/Neal/pebble-vlc-remote/master/html/configuration.html?' +
// 				'server_host=' + encodeURIComponent(server_host) +
// 				'&server_pass=' + encodeURIComponent(server_pass);
// 	console.log('showing configuration at uri: ' + uri);
// 	Pebble.openURL(uri);
// });

// Pebble.addEventListener('webviewclosed', function(e) {
// 	console.log('configuration closed');
// 	if (e.response) {
// 		var options = JSON.parse(decodeURIComponent(e.response));
// 		console.log('options received from configuration: ' + JSON.stringify(options));
// 		localStorage.setItem('server_host', options['server_host']);
// 		localStorage.setItem('server_pass', options['server_pass']);
// 		makeRequestToVLC(options['server_host'], options['server_pass'], 'refresh');
// 	} else {
// 		console.log('no options received');
// 	}
// });








// // Fetch saved symbol from local storage (using standard localStorage webAPI)
// // var symbol = localStorage.getItem("symbol");

// // // We use the fake "PBL" symbol as default
// // if (!symbol) {
// //   symbol = "PBL";
// // }

// function fetchAuthKey() {
//   var response;
//   var req = new XMLHttpRequest();
//   console.log('Getting pres info');
//   // build the GET request
//   var info = localStorage.getItem("auth_key") ? localStorage.getItem("auth_key") : '';
//   req.open('GET', "http://pebblepresenter.syvarth.com/getPresentationPebble/"+info, true);
//   req.onload = function(e) {
//   	console.log(req.readyState);
// 	if (req.readyState == 4) {
// 	  // 200 - HTTP OK
// 	  if(req.status == 200) {
// 		console.log(req.responseText);
// 		response = JSON.parse(req.responseText);
// 		if (response.auth_key) {
// 		    Pebble.sendAppMessage({symbol: response.auth_key});
// 		    localStorage.setItem("auth_key", response.auth_key);
// 		    localStorage.setItem("pebble_id", response.pebble_id);
// 		} else if( response.pres_id ) {
// 			Pebble.sendAppMessage({symbol: 10});//Number of slides
// 		}
// 	  } else {
// 		console.log("Request returned error code " + req.status.toString());
// 	  }
// 	}
//   }
//   req.send(null);
// }

// function changeSlide(direction) {
//   var response;
//   var req = new XMLHttpRequest();
//   console.log('Change the slide');
//   console.log(direction);
//   var direct = (direction == -1) ? 'next' : 'back';
//   // build the GET request
//   req.open('GET', "http://pebblepresenter.syvarth.com/changeSlide/"+localStorage.getItem("pebble_id")+"/"+direct, true);
//   req.onload = function(e) {
// 	if (req.readyState == 4) {
// 	  // 200 - HTTP OK
// 	  if(req.status == 200) {
// 		console.log(req.responseText);
// 		Pebble.sendAppMessage({price: 0});
// 	  } else {
// 		console.log("Request returned error code " + req.status.toString());
// 	  }
// 	}
//   }
//   req.send(null);
// }

// // Set callback for the app ready event
// Pebble.addEventListener("ready",
// 						function(e) {
// 						  console.log("connect!" + e.ready);
// 						  console.log(e.type);
// 						});

// // Set callback for appmessage events
// Pebble.addEventListener("appmessage",
// 						function(e) {
// 						  console.log("message caught");
// 						  console.log(e.payload.price);
// 						  console.log(e.payload.symbol);
// 						  var num = e.payload.price;

// 						  if( num == 1 || num == -1 ) { // GetAuthKey / getSlides
// 						  	  changeSlide(num);
// 						  }

// 						   if( e.payload.symbol == 'refr' ) { // changeSlide
// 						  	  fetchAuthKey();
// 						   }

// 						});

