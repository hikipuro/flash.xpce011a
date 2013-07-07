package  
{
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.text.TextField;
	import xpce.vfs.ISpecialFile;
	import xpce.CModule;
	
	/**
	 * SWC 側からの printf を表示するためのクラス.
	 * @author Hikipuro
	 */
	public class SwcConsole extends Sprite implements ISpecialFile 
	{
		private var _textField:TextField;
		
		public function SwcConsole() {
			addEventListener(Event.ADDED_TO_STAGE, function ():void {
				_textField = new TextField();
				_textField.width = stage.stageWidth;
				_textField.height = stage.stageHeight;
				addChild(_textField);
			});
		}
		
        /**
         * The PlayerKernel implementation will use this function to handle
         * C IO write requests to the file "/dev/tty" (e.g. output from
         * printf will pass through this function). See the ISpecialFile
         * documentation for more information about the arguments and return value.
         */
        public function write(fd:int, bufPtr:int, nbyte:int, errnoPtr:int):int {
            var str:String = CModule.readString(bufPtr, nbyte);
			if (_textField) {
				//tf.appendText(str);
			}
            trace(str.substr(0, -1));
            return nbyte;
        }
		
        /** See ISpecialFile */
        public function read(fd:int, bufPtr:int, nbyte:int, errnoPtr:int):int { return 0; }
        public function fcntl(fd:int, com:int, data:int, errnoPtr:int):int { return 0; }
        public function ioctl(fd:int, com:int, data:int, errnoPtr:int):int { return 0; }
	}

}