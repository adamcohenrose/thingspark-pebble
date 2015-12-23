// Called when JS is ready
Pebble.addEventListener("ready", function(e) {
  console.log("PebbleKit JS ready");
});

Pebble.addEventListener('showConfiguration', function(e) {
  var url = 'http://thingspark.cohen-rose.org/';
  console.log("Showing configuration page: " + url);
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var settings = JSON.parse(decodeURIComponent(e.response));
  settings.msgType = 'settings';
  console.log("Configuration page returned: " + JSON.stringify(settings));

  Pebble.sendAppMessage(settings, function() {
    console.log('Send successful!');
  }, function() {
    console.log('Send failed!');
  });
});

// Called when incoming message from the Pebble is received
Pebble.addEventListener("appmessage", function(e) {
  var channelId = e.payload.channelId;
  var fieldNum = e.payload.fieldNum;
  var chartHeight = e.payload.chartHeight;
  var apiKey = e.payload.apiKey;
  var graphWidth = e.payload.graphWidth;
  requestFeed(channelId, fieldNum, chartHeight, apiKey, graphWidth);
});

function requestFeed(channelId, fieldNum, chartHeight, apiKey, graphWidth) {
  var req = new XMLHttpRequest();
  var url = 'https://api.thingspeak.com/channels/' 
          + channelId + '/fields/' + fieldNum + '.json?results=' + graphWidth;
  if (apiKey) {
    url += '&api_key=' + apiKey;
  }
  console.log("fetching feed from: " + url);

  req.open('GET', url);
  req.onload = function(e) {
    // prepare response for Pebble app
    var msg = {
      'msgType': 'data',
      'channelId': channelId,
      'fieldNum': fieldNum,
      'data': "",
    };
    if (req.readyState == 4 && req.status == 200) {
      var response = JSON.parse(req.responseText);
      console.log("reading feed for " 
        + response.channel.name + " / " + response.channel['field' + fieldNum]);
      var rawData = new Array();
      var datapoints = response.feeds;
      for (var idx = 0; idx < datapoints.length; idx++) {
        rawData.push(parseFloat(datapoints[idx]['field' + fieldNum]));
        //console.log(datapoints[idx]['field' + fieldNum] + " @ " + datapoints[idx].created_at);
      }

      // calculate height-adjusted values
      var valueMin = Math.min.apply(null, rawData);
      var valueMax = Math.max.apply(null, rawData);
      var step = (valueMax - valueMin) / (chartHeight - 1.0);

      var scaledValue = 0;
      for (idx = 0; idx < rawData.length; idx++) {
        scaledValue = Math.round((rawData[idx] - valueMin) / step);
        msg.data += String.fromCharCode(scaledValue + 65);
        //console.log(rawData[idx] + " @ " + scaledValue);
      }
      msg.value = rawData[rawData.length - 1].toFixed(1);
      msg.valueMin = valueMin.toFixed(1);
      msg.valueMax = valueMax.toFixed(1);
      
    } else {
      msg.value = 'N/A';
      msg.valueMin = msg.valueMax = 'error';
    }

    Pebble.sendAppMessage(msg);
  };
  req.send();
}
