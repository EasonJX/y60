plug( "EventLoop" );
plug( "ASSEventSource" );
plug( "SynergyServer" );

var _mySynergyServer = new SynergyServer( "localhost", 24800 );

var myASSEventSource = new ASSEventSource();
var mySettings = new Node();
mySettings.parseFile( "settings.xml" );
myASSEventSource.onUpdateSettings( mySettings );

// wait until we have a connection with the synergy client
while (!_mySynergyServer.isConnected()) {
    ;
}


const MOVE_VELOCITY = 100; 
const RESIZE_VELOCITY = 20; 
const DAMPING_FACTOR = 1.3; 

const POSITION_OFFSET = [0,140];
var _myScreenSize = _mySynergyServer.getScreenSize();

var _myMouseMoveId = null;
var _myButtonPressedId = null;
var _myTargetPosition = null;
var _myVelocity = new Vector2f(0,0);
var _myMousePosition = new Vector2f( 0, 0 );
var _myGridSize = null;
var _myMirror = new Vector2i(0,0);

var myMirrorXNode = mySettings.find( "//MirrorX" );
if (myMirrorXNode) {
    _myMirror.x = myMirrorXNode.childNode("#text").nodeValue;
}

var myMirrorYNode = mySettings.find( "//MirrorY" );
if (myMirrorYNode) {
    _myMirror.y = myMirrorYNode.childNode("#text").nodeValue;
}


function onFrame ( theTime ) {

    if (_myTargetPosition) {

        var myPosition = product( sum( product( _myMousePosition, 
                                                MOVE_VELOCITY), 
                                       _myTargetPosition), 
                                  1 / (MOVE_VELOCITY + 1)); 
        _myMousePosition = myPosition; 
        if (distance( _myMousePosition, _myTargetPosition ) < 0.1) {
            _myTargetPosition = null;
        }
        _mySynergyServer.onMouseMotion( _myMousePosition.x + POSITION_OFFSET[0], 
                                        _myMousePosition.y + POSITION_OFFSET[1] );

    }

}

function getMousePos( theRawPosition ) {
    var myRawX = theRawPosition.x;
    if (_myMirror.x > 0) {
        myRawX = _myGridSize.x - theRawPosition.x;
    }
    var myX = myRawX / _myGridSize.x 
              * _myScreenSize.x;
              
    var myRawY = theRawPosition.y;
    if (_myMirror.y > 0) {
        myRawY = _myGridSize.y - theRawPosition.y;
    }
              
    var myY = myRawY / _myGridSize.y 
              * _myScreenSize.y;

    return new Vector2f( myX, myY );
}


function onASSEvent ( theEvent ) {
    
    print( theEvent );

    if ( theEvent.type == "add") {
        if (_myMouseMoveId == null) {
            _myMouseMoveId = theEvent.id;
            _myTargetPosition = null;
            if (_myGridSize) {
                var myCurMousePos = getMousePos( theEvent.raw_position );
                if (distance( _myMousePosition, myCurMousePos ) < 100) {
                    _myMousePosition = myCurrentMousePosition;
                }
            }
            _myVelocity = new Vector2f( 0, 0 );
        } else if (_myButtonPressedId == null) {
            _myButtonPressedId = theEvent.id;
            _mySynergyServer.onMouseButton( 1, true );
        }
    } else if ( theEvent.type == "remove") {
        if (_myMouseMoveId == theEvent.id) {
            _myMouseMoveId = null;
        } else if (_myButtonPressedId == theEvent.id) {
            _myButtonPressedId = null;
            _mySynergyServer.onMouseButton( 1, false );
        }
    } else if ( theEvent.type == "move" ) {
        if (_myMouseMoveId == theEvent.id) {
            if (_myGridSize) {
                _myTargetPosition = getMousePos( theEvent.raw_position );
            }
        }
    } else if ( theEvent.type == "configure" ) {
        _myGridSize = theEvent.grid_size;
    }

}


var myEventLoop = new EventLoop();
myEventLoop.go(this);

