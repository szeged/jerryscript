var http = require('http');
var formidable = require('formidable');
var fs = require('fs');
var querystring = require('querystring');

function processPost(request, response, callback) {
    var queryData = "";
    if(typeof callback !== 'function') return null;

    if(request.method == 'POST') {
        request.on('data', function(data) {
            queryData += data;
            if(queryData.length > 1e6) {
                queryData = "";
                response.writeHead(413, {'Content-Type': 'text/plain'}).end();
                request.connection.destroy();
            }
        });

        request.on('end', function() {
            request.post = querystring.parse(queryData);
            callback();
        });

    } else {
        response.writeHead(405, {'Content-Type': 'text/plain'});
        response.end();
    }
}

var message_state = {
  nothing : 0,
  fail : 1,
  success :2
}
var default_value = JSON.stringify ({get_interval : 10 * 1000 * 60,
                                    pic_interval : 10 * 1000 * 60,
                                    measure_interval : 10 * 1000 * 60,
                                    data_send_interval : 1000 * 60 * 60,
                                    delete_after_send : true});

var cssFileName = "bootstrap.min.css";
var configFileName = "config.json";

var cssFile = fs.readFileSync(cssFileName, "utf8");
var popup_message = message_state.nothing;

if (!fs.existsSync(configFileName)) {
  fs.writeFileSync(configFileName, default_value, 'utf-8', function (err) {
    if (err) {
      response.send("failed open or creat config file");
    }
  });
}
var configFile = fs.readFileSync(configFileName, "utf8");
var actualConfig = JSON.parse(configFile);

http.createServer(function (request, response) {
  if(request.method == 'POST') {
    console.log(request.post)
        processPost(request, response, function() {

          if (!fs.existsSync(configFileName)) {
            fs.writeFileSync(configFileName, default_value, 'utf-8', function (err) {
              if (err) {
                response.send("failed open or creat config file");
              }
            });
          }
          configFile = fs.readFileSync(configFileName, "utf8");

          actualConfig = JSON.parse(configFile);
          var multiplier = {};
          var skip_unit = false;
          var set_delete_to_false = true;
          for (const [key, value] of Object.entries(request.post)) {
            if (skip_unit){
              skip_unit = false;
              continue;
            }
            var splitted = key.split("_unit");
            if (actualConfig.hasOwnProperty(splitted[0])){
              if (splitted.length == 1){
                var numValue = Number(value);
                if (!isNaN(numValue) && numValue > 0)
                {
                  actualConfig[splitted[0]] = numValue;
                }
                else if (value == "on"){
                  set_delete_to_false = false;
                  actualConfig[splitted[0]] = true;
                  continue;
                }
                else if (value.length == 0){
                  skip_unit = true;
                  continue;
                }
                else{
                  popup_message = message_state.fail;
                  break;
                }
              }
              else{
                switch (value) {
                  case "millisecond":
                    multiplier[splitted[0]] = 1;
                    break;
                  case "second":
                    multiplier[splitted[0]] = 1000;
                    break;
                  case "minute":
                    multiplier[splitted[0]] = 1000 * 60;
                    break;
                  case "hour":
                    multiplier[splitted[0]] = 1000 * 60 * 60;
                    break;
                  default:
                    popup_message = message_state.fail;
                    break;
                }
              }
            }
            else {
              popup_message = message_state.fail;
              break;
            }
          }

          for (const [key, value] of Object.entries(actualConfig)) {
            console.log(key, value, actualConfig[key],multiplier[key])
            if (multiplier.hasOwnProperty (key)){
              actualConfig[key] *= multiplier[key];
            }
          }

          if (set_delete_to_false){
            actualConfig.delete_after_send = false;
          }


          if (popup_message != message_state.fail){
            fs.writeFile(configFileName, JSON.stringify(actualConfig), function(err) {
                if(err) {
                  return console.log(err);
                }
            });
            popup_message = message_state.success;
          }
          response.writeHead(301, {Location: 'http://localhost:8001/'});
          response.end();
        });


    }
    else {
      switch (request.url) {
        case "/" + cssFileName :
            response.writeHead(200, {"Content-Type": "text/css"});
            response.write(cssFile);
            break;
        default :
            response.writeHead(200, "OK", {'Content-Type': 'text/html'});
            response.write('<link rel="stylesheet" type="text/css" href="' + cssFileName + '">');
            if (popup_message == message_state.fail){
              response.write (
              '<div class="alert alert-danger alert-dismissible">' +
              '<a href="" class="close" data-dismiss="alert" aria-label="close">&times;</a>' +
              '<strong>Invalid data!</strong> The configuration has not changed!</div>');
              popup_message = message_state.nothing;
            }
            else if (popup_message == message_state.success){
              response.write (
              '<div class="alert alert-success alert-dismissible">' +
              '<a href="" class="close" data-dismiss="alert" aria-label="close">&times;</a>' +
              '<strong>Success!</strong> The configuration has changed!</div>');
              popup_message = message_state.nothing;
            }
            response.write('<div class="container">');
            response.write('<h1>ESP8266 config page</h1>');
            response.write(
            '<form action="" method="POST" role="form">' +
              '<div class="form-group">' +
                '<label class= "control-label col-md-5">Config update interval</label>' +
                '<div class="input-group col-md-7">' +
                  '<input type="number" class="form-control" style="margin-right: 20;" id="get_interval" name="get_interval" placeholder="Enter config update interval">' +
                  '<select class="form-control" name="get_interval_unit">' +
                    '<option>hour</option>' +
                    '<option>minute</option>' +
                    '<option>second</option>' +
                    '<option>millisecond</option>' +
                  '</select>' +
                '</div>' +
              '</div>' +
              '<div class="form-group mb-2">' +
                '<label class= "control-label col-md-5">Picture take interval</label>' +
                '<div class="input-group col-md-7">' +
                  '<input type="number" class="form-control" style="margin-right: 20;" id="pic_interval" name="pic_interval" placeholder="Enter picture take interval">' +
                  '<select class="form-control" name="pic_interval_unit">' +
                    '<option>hour</option>' +
                    '<option>minute</option>' +
                    '<option>second</option>' +
                    '<option>millisecond</option>' +
                  '</select>' +
                '</div>' +
              '</div>' +
              '<div class="form-group mb-2">' +
                '<label class= "control-label col-md-5">Sensor measure interval</label>' +
                '<div class="input-group col-md-7">' +
                  '<input type="number" class="form-control" style ="margin-right: 20;" id="measure_interval" name="measure_interval" placeholder="Enter sensor measure interval">' +
                  '<select class="form-control" name="measure_interval_unit">' +
                    '<option>hour</option>' +
                    '<option>minute</option>' +
                    '<option>second</option>' +
                    '<option>millisecond</option>' +
                  '</select>' +
                '</div>' +
              '</div>' +
              '<div class="form-group mb-2">' +
                '<label class= "control-label col-md-5">Data send interval</label>' +
                '<div class="input-group col-md-7">' +
                  '<input type="number" class="form-control" style ="margin-right: 20;" id="data_send_interval" name="data_send_interval" placeholder="Enter data send interval">' +
                  '<select class="form-control" name="data_send_interval_unit">' +
                    '<option>hour</option>' +
                    '<option>minute</option>' +
                    '<option>second</option>' +
                    '<option>millisecond</option>' +
                  '</select>' +
                '</div>' +
              '</div>' +
              '<div class="form-group mb-2">' +
                '<label class="control-label col-md-5">Delete after send</label>' +
                '<label><input type="checkbox" name="delete_after_send"' + (actualConfig.delete_after_send ? 'checked' : '') +'>On</label>' +
              '</div>' +
              '<div class = "col-md-7" style="text-align: right;"><button type="submit" class="btn btn-primary">Submit</button></div>' +
            '</form> </div>'
          );
    };
    response.end();
    }
}).listen(8001);
