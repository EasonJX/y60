use("HttpClient.js");

var Hoccer = {};



Hoccer.station = function() {
    var that = {};
    var myRequestManager = new RequestManager();
    var myOnErrorFunc = function () {
            Logger.warning( "HTTP Code received: " 
                            + this.responseCode); 
        };

    that.userAgent = "Hoccer/0.9dev Y60";
    that.serverUri = "http://beta.hoccer.com";

    //default hoccer station at artcom
    that.longitude = 13.345116;
    that.latitude = 52.501077;
    that.accuracy = 80;
    
    that.upload = function(theFile, theUploadUri) {
        print("try to upload ", theFile, " to ", theUploadUri);
        var border = "ycKtoN8VURwvDC4sUzYC9Mo7l0IVUyDDVf";
        var request = new Request(theUploadUri, that.userAgent);
        request.addHttpHeader("Content-Type", "multipart/form-data; boundary=" + border);
        var body = "--" + border + "\r\n" +
        "Content-Disposition: form-data; name=\"" + "upload[attachment]" + "\" "+
        "filename=\"" + theFile + "\"\r\n" +
        "Content-Type: " + "text/plain" + "\r\n" +
        "Content-Transfer-Encoding: binary\r\n\r\n";

        body += "Hallo Welt.";

        body += "\r\n--" + border + "--\r\n";

        print("this will be send: ", body);

        request.onDone = function() {
            print("upload done. handle response: ", this.responseString, "  code: ", this.responseCode);
        };
        request.onError = myOnErrorFunc;

        request.put(body);
        myRequestManager.performRequest(request);
    };

    that.download = function(theDownloadUri) {
        print("download ", theDownloadUri);
        var request = new Request(theDownloadUri, that.userAgent);
        request.onDone = function() {
            print("download done. handle response: ", this.responseString, "  code: ", this.responseCode);
        };
        request.onError = myOnErrorFunc;
        request.get();
        myRequestManager.performRequest(request);
    };

    that.prepareDownload = function(thePeerUri) {
        print("prepare Download from peer uri ", thePeerUri);
        var request = new Request(thePeerUri, that.userAgent);
        request.onDone = function() {
            print("prepare download done. handle response: ", this.responseString, "  code: ", this.responseCode);
            var response = eval("("+this.responseString+")");
            var resources = response.resources;
            var downloadUri = "";
            if (resources.length > 0) {
                downloadUri = resources[0];
            }
            if (downloadUri.length > 0) {
                that.download(downloadUri);
            }
        };
        request.onError = myOnErrorFunc;
        request.get();
        myRequestManager.performRequest(request);
    };

    that.buildPeerGroup = function(theParams) {
        print("buildPeerGroup");
        if (typeof (theParams.isSharing) == 'undefined') {
            theParams.isSharing = false;
        }
        var request = new Request(that.serverUri + "/peers",  that.userAgent); 
        request.onDone = (typeof (theParams.onDone)=='undefined'?function(){}:theParams.onDone);
        request.onError = (typeof (theParams.onError)=='undefined'?function(){}:theParams.onError);

        var body = "peer[gesture]=distribute" +
                    "&peer[latitude]=" + that.latitude +
                    "&peer[longitude]=" + that.longitude +
                    "&peer[accuracy]=" + that.accuracy + 
                    (theParams.isSharing?"&peer[seeder]=1":"");
        request.post(body);
        myRequestManager.performRequest(request );
        print("posted ",body);
    };



    that.distribute = function(theFile) {
        print("distribute");
        
        that.buildPeerGroup({
           isSharing : true,
           onDone : function() {
               print("build peergroup for distribute done. handle response: ", this.responseString, "  code: ", this.responseCode);
               var response = eval("("+this.responseString+")");
               var uploadUri = response.upload_uri;
               print("uploadUri: ", uploadUri);
               that.upload(theFile, uploadUri);
           },
           onError : myOnErrorFunc
         });
    };

    that.catchIt = function() {
        print("catch it");

        that.buildPeerGroup({
            isSharing : false,
            onDone : function() {
               print("build peergroup for catch it done. handle response: ", this.responseString, "  code: ", this.responseCode);
               var response = eval("("+this.responseString+")");
               var peerUri = response.peer_uri;
               that.prepareDownload(peerUri);
            },
            onError : myOnErrorFunc
        });
    };

    that.update = function() {
        myRequestManager.handleRequests();
    };



    return that;
}();

