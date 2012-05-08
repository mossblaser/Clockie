class Unicom {
	static var encoder : Unicom;
	
	static var dataGenerators;
	
	// entry point
	static function main(mc) {
		initBox();
		_root.bg.onRelease = onRelease;
		
		encoder = new Unicom();
		_root.onEnterFrame = onEnterFrame;
	}
	
	
	static function onRelease() {
		if (encoder.isReady())
			stopTransmission();
		else
			startTransmission();
	}
	
	
	static function startTransmission() {
		encoder.reset();
		
		dataGenerators = [
			sendSync,
			sendTime,
			sendMeetup
		];
	}
	
	static function stopTransmission() {
		encoder.reset();
		dataGenerators = [];
	}
	
	static function sendSync() {
		encoder.addSyncInterval(40);
	}
	
	static function sendTime() {
		var now : Date = new Date();
		encoder.addBigEndian((now.getTime()/1000) - (60*now.getTimezoneOffset()), 4);
	}
	
	static function sendMeetup() {
		var year  = _root.year;
		var month = _root.month;
		var day   = _root.day;
		var hour  = _root.hour;
		var min   = _root.min;
		var sec   = _root.sec;
		
		var meeting : Date = new Date(year,month - 1,day,hour,min,sec);
		encoder.addBigEndian((meeting.getTime()/1000) - (60*meeting.getTimezoneOffset()), 4);
	}
	
	
	static function onEnterFrame() {
		if ((!encoder.isReady()) && dataGenerators.length > 0)
			dataGenerators.shift()();
		
		if (encoder.isReady())
			setColour(encoder.pop() ? 0xFFFFFF : 0x000000);
	}
	
	
	static var colour : Color;
	
	static function initBox() {
		_root.createEmptyMovieClip("bg",0);
		
		_root.bg.beginFill(0x000000);
		_root.bg.moveTo(0,0);
		_root.bg.lineTo(Stage.width,0);
		_root.bg.lineTo(Stage.width,Stage.height);
		_root.bg.lineTo(0,Stage.height);
		_root.bg.endFill();
		
		colour = new Color("_root.bg");
	}
	
	static function setColour(val) {
		colour.setRGB(val);
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	
	
	var data;
	var clock;
	
	// Constructor
	function Unicom() {
		reset();
	}
	
	
	function reset() {
		// Empty stream
		data = [];
		
		// Initial clock bit
		clock = 0;
	}
	
	
	function pop() {
		// Toggle the clock
		clock = clock ^ 1;
		
		if (clock) {
			return !data[0];
		} else {
			return data.shift();
		}
	}
	
	
	function isReady() {
		return data.length != 0;
	}
	
	
	function addSyncInterval(duration) {
		// Send 1s for synchromnisation
		for (var i = 0; i < duration; i++)
			data.push(1);
		
		// Send a 0 to indicate beginning of transmission
		data.push(0);
	}
	
	
	function addByte(value) {
		for (var i = 1; i <= 8; i++)
			data.push((value>>(8-i)) & 1);
	}
	
	
	function addBigEndian(value, length) {
		for (var i = 0; i < length; i++)
			addByte((value>>(i*8)) & 0xFF);
	}
}
