// From Getting Started With node.js and socket.io 
// http://codehenge.net/blog/2011/12/getting-started-with-node-js-and-socket-io-v0-7-part-2/
"use strict";

var http = require('http'),
    url = require('url'),
    fs = require('fs'),
    exec = require('child_process').exec,
    server,
    connectCount = 0;	// Number of connections to server

server = http.createServer(function (req, res) {
// server code
    var path = url.parse(req.url).pathname;
    console.log("path: " + path);
    switch (path) {
    case '/':
        res.writeHead(200, {'Content-Type': 'text/html'});
        res.write('<h1>Hello!</h1>Try<ul><!--<li><a href="/buttonBox.html">Button Box Demo</a></li>--><li><a href="/accelData.html">AccelData</a></li></ul>');

        res.end();
        break;

    default:		// This is so all the files will be sent.
        fs.readFile(__dirname + path, function (err, data) {
            if (err) {return send404(res); }
//            console.log("path2: " + path);
            res.write(data, 'utf8');
            res.end();
        });
        break;

    }
});

var send404 = function (res) {
    res.writeHead(404);
    res.write('404');
    res.end();
};

server.listen(8081);

// socket.io, I choose you
var io = require('socket.io').listen(server);
io.set('log level', 2);

// on a 'connection' event
io.sockets.on('connection', function (socket) {
    var frameCount = 0;	// Counts the frames from arecord
    var lastFrame = 0;	// Last frame sent to browser
    console.log("Connection " + socket.id + " accepted.");
//    console.log("socket: " + socket);

    // now that we have our connected 'socket' object, we can 
    // define its event handlers

    // Make sure some needed files are there
    // The path to the analog devices changed from A5 to A6.  Check both.
    var ainPath = "/sys/devices/platform/omap/tsc/";
    
//    if(!fs.existsSync(ainPath)) {
//        ainPath = "/sys/devices/platform/tsc/";
//        if(!fs.existsSync(ainPath)) {
//            throw "Can't find " + ainPath;
//        }
//    }
    // Make sure gpio 7 is available.
    //exec("echo 7 > /sys/class/gpio/export");

    var ainNum1 = 1;
    var ainNum2 = 3;
    var ainNum3 = 7;
    var pushInterval = 100;

    function pushData() {
	fs.readFile(ainPath + "ain" + ainNum1, 'base64', function(err, data) {
    	    if(err) throw err;
	    socket.emit('ain1', data.substr(0,4));
//	    console.log('emitted ain1: ' + data.substr(0,4));
	});

	fs.readFile(ainPath + "ain" + ainNum2, 'base64', function(err, data) {
	    if(err) throw err;
	    socket.emit('ain2', data.substr(0,4));
//            console.log('emitted ain2: ' + data.substr(0,4));
	});

	fs.readFile(ainPath + "ain" + ainNum3, 'base64', function(err, data) {
	    if(err) throw err;
	    socket.emit('ain3', data.substr(0,4));
	//            console.log('emitted ain3: ' + data.substr(0,4));
	});
	setTimeout(pushData, pushInterval);
    }

    socket.on('updateain1', function (ainNum) {
	ainNum1 = ainNum;
	//console.log('Updated ainNum1: ' + ainNum1);
    });

    socket.on('updateain2', function (ainNum) {
	ainNum2 = ainNum;
	//console.log('Updated ainNum2: ' + ainNum1);
    });

    socket.on('updateain3', function (ainNum) {
	ainNum3 = ainNum;
	//console.log('Updated ainNum3: ' + ainNum1);
    });

    socket.on('updateTimer', function (timer) {
	pushInterval = timer;
	console.log('New push interval: ' + pushInterval);
    });
/*
    // Send value every time a 'message' is received.
    socket.on('ain1', function (ainNum) {
//        var ainPath = "/sys/devices/platform/omap/tsc/ain" + ainNum;
        fs.readFile(ainPath + "ain" + ainNum, 'base64', function(err, data) {
            if(err) throw err;
            socket.emit('ain1', data.substr(0,4));
//            console.log('emitted ain1: ' + data.substr(0,4));
        });
    });
*/
    socket.on('disconnect', function () {
        console.log("Connection " + socket.id + " terminated.");
        connectCount--;
        if(connectCount === 0) {
        }
        console.log("connectCount = " + connectCount);
    });

    connectCount++;
    console.log("connectCount = " + connectCount);

    pushData();
});

