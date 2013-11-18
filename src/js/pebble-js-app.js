var maxTriesForSendingAppMessage = 3;
var timeoutForAppMessageRetry = 3000;
var timeoutForRequest = 20000;

var slides = [
	{ time: 20 },
	{ time: 30 },
	{ time: 40 },
	{ time: 50 },
	{ time: 60 },
	{ time: 70 },
	{ time: 80 }
];

var currSlide = 0;

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
						'auth': res.auth_key,
						'time': slides[ currSlide ].time
					});
				} else if( res.auth_key ) {
					sendAppMessage({
						'auth': res.auth_key,
						'time': slides[ currSlide ].time
					});

					localStorage.setItem("auth_key", res.auth_key);
 	            	localStorage.setItem("pebble_id", res.pebble_id);
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

				currSlide = direction == 'next' ? currSlide++ : currSlide--;
				currSlide = currSlide > slides.length ? slides.length : currSlide;
				currSlide = currSlide < 0 ? 0 : currSlide;

				sendAppMessage({
					'time': slides[ currSlide ].time
				});
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


Pebble.addEventListener('ready', function(e) {});

Pebble.addEventListener('appmessage', function(e) {
	console.log('AppMessage received from Pebble: ' + JSON.stringify(e.payload));


	switch (e.payload.request) {
		case 'next':
			changeSlide('next');
			break;
		case 'prev':
			changeSlide('prev');
			break;
		case 'refresh':
			var pebble_id = localStorage.getItem("pebble_id") ? localStorage.getItem("pebble_id") : '';

			fetchAuthKey( pebble_id );
			break;
		default:
			console.log('Command not recognized');
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


