package 
{
	import com.bit101.components.Label;
	import com.bit101.components.PushButton;
	import flash.display.Bitmap;
	import flash.display.BitmapData;
	import flash.display.Sprite;
	import flash.display.StageAlign;
	import flash.display.StageScaleMode;
	import flash.events.Event;
	import flash.events.SampleDataEvent;
	import flash.media.Sound;
	import flash.net.FileFilter;
	import flash.net.FileReference;
	import flash.ui.Keyboard;
	import flash.utils.ByteArray;
	import flash.utils.Endian;
	import xpce.CModule;
	import xpce.getBytes;
	import xpce.getSoundBytes;
	import xpce.loadRom;
	import xpce.nextFrame;
	import xpce.setKeys;
	
	/**
	 * xpce の Flash 側のソース.
	 * @author Hikipuro
	 */
	public class Main extends Sprite
	{
		/** 初期表示サイズ. */
		private const _DEFAULT_SCALE:int = 3;
		
		private const _WIDTH:int = 352;
		private const _HEIGHT:int = 224;
		
		/** xpce 側から渡されたデータを描画するためのオブジェクト. */
		private var _bitmapData:BitmapData;
		
		/** _bitmapData を画面に表示するためのオブジェクト. */
		private var _bitmap:Bitmap;
		
		/** キー入力処理用. */
		private var _keyInput:KeyInput;
		
		/** ファイル選択用. */
		private var _fileReference:FileReference;
		
		/** 枠線表示用. */
		private var _background:Sprite;
		
		/**
		 * コンストラクタ.
		 */
		public function Main():void {
			if (stage) _init();
			else addEventListener(Event.ADDED_TO_STAGE, _init);
		}
		
		/**
		 * 初期化メソッド.
		 * @param	e
		 */
		private function _init(e:Event = null):void {
			removeEventListener(Event.ADDED_TO_STAGE, init);
			// entry point
			
			// 画面表示用の設定
			stage.scaleMode = StageScaleMode.NO_SCALE;
			stage.align = StageAlign.TOP_LEFT;
			
			// ファイル選択ダイアログの初期化
			_fileReference = new FileReference();
			_fileReference.addEventListener(Event.SELECT, _onFileSelect);
			
			// UI パーツを作成する
			_initUI();
			
			//
			_initXpce();
			
			// 初期表示サイズで表示
			_setScale(_DEFAULT_SCALE);
		}
		
		private function _initXpce():void {
			// SWC 側で printf したメッセージを表示するための準備
			var swcConsole:SwcConsole = new SwcConsole();
			//swcConsole.y = 200;
			//addChild(swcConsole);
			CModule.vfs.console = swcConsole;
			
			// SWC の main を開始
			CModule.startAsync(swcConsole);
			
			// xpce を初期化
			//initTgbDual();
			
			// 描画領域を準備
			var bytes:ByteArray = getBytes();
			bytes.endian = Endian.LITTLE_ENDIAN;
			bytes.position = 0;
			
			_bitmapData = new BitmapData(_WIDTH, _HEIGHT);
			_bitmapData.setPixels(_bitmapData.rect, bytes);
			_bitmap = new Bitmap(_bitmapData);
			_bitmap.x = 10;
			_bitmap.y = 40;
			_bitmap.smoothing = true;
			addChild(_bitmap);
			
			// キー入力処理用のオブジェクトを初期化
			_keyInput = new KeyInput();
			addChild(_keyInput);
			
			// xpce から受け取ったサウンドを再生するためのオブジェクトを初期化
			var sound:Sound = new Sound();
			sound.addEventListener(SampleDataEvent.SAMPLE_DATA, _onSampleData);
			sound.play(0, 1);
			
			// 毎フレーム処理するためのイベントハンドラを登録
			addEventListener(Event.ENTER_FRAME, _onEnterFrame);
		}
		
		/**
		 * UI パーツを作成する.
		 */
		private function _initUI():void {
			// 枠線表示用
			_background = new Sprite();
			_background.x = 10;
			_background.y = 40;
			addChild(_background);
			
			// Load ROM ボタン
			var loadButton:PushButton = new PushButton(this, 10, 10, 'Load ROM', function ():void {
				_fileReference.browse([
					new FileFilter("*.pce", "*.pce")
				]);
			});
			loadButton.width = 80;
			
			// Scale ラベル
			var scaleLabel:Label = new Label(this, 110, 10, 'Scale');
			
			// x1 ボタン
			var x1Button:PushButton = new PushButton(this, 150, 10, 'x1', function ():void {
				_setScale(1);
			});
			x1Button.width = 40;
			
			// x2 ボタン
			var x2Button:PushButton = new PushButton(this, 200, 10, 'x2', function ():void {
				_setScale(2);
			});
			x2Button.width = 40;
			
			// x3 ボタン
			var x3Button:PushButton = new PushButton(this, 250, 10, 'x3', function ():void {
				_setScale(3);
			});
			x3Button.width = 40;
			
			// max ボタン
			var maxButton:PushButton = new PushButton(this, 300, 10, 'max', function ():void {
				_bitmap.height = stage.stageHeight - _bitmap.y;
				_bitmap.scaleX = _bitmap.scaleY;
				
				_background.graphics.clear();
				_background.graphics.lineStyle(1);
				_background.graphics.drawRect(0, 0, _bitmap.width - 1, _bitmap.height - 1);
			});
			maxButton.width = 40;
			
			// 使い方ラベル
			var howToLabel:Label = new Label(this, 370, 10, 'II: Z Key, I: X Key, Select: Shift Key, Run: Enter Key');
		}
		
		/**
		 * 表示サイズを設定する.
		 * @param	scale 1: 等倍, 2: 2 倍.
		 */
		private function _setScale(scale:int):void {
			_bitmap.scaleX = scale;
			_bitmap.scaleY = scale;
			_redrawBorder(scale);
		}
		
		/**
		 * 枠線を再描画する.
		 * @param	scale 1: 等倍, 2: 2 倍.
		 */
		private function _redrawBorder(scale:int):void {
			_background.graphics.clear();
			_background.graphics.lineStyle(1);
			_background.graphics.drawRect(0, 0, _WIDTH * scale - 1, _HEIGHT * scale - 1);
		}
		
		/**
		 * ファイル選択ダイアログでファイルが選択された時のイベントハンドラ.
		 * @param	e
		 */
		private function _onFileSelect(e:Event):void {
			_fileReference.addEventListener(Event.COMPLETE, _onFileLoadComplete);
			_fileReference.load();
		}
		
		/**
		 * ファイルのロードが完了した時のイベントハンドラ.
		 * @param	e
		 */
		private function _onFileLoadComplete(e:Event):void {
			var data:ByteArray = _fileReference.data;
			var offset:int = data.length % 0x1000;
			data.position = offset;
			trace("offset: " + offset);
			loadRom(0, data);
		}
		
		/**
		 * サウンドデータが足りなくなった時に Flash から呼び出される.
		 * @param	e
		 */
		private function _onSampleData(e:SampleDataEvent):void {
			//return;
			//trace("_onSampleData");
			var data:ByteArray = e.data;
			
			// xpce からサウンドデータを取得
			var bytes:ByteArray = getSoundBytes();
			
			// サウンドデータが空だった時は無音を再生する
			if (bytes == null) {
				for (var n:uint = 0; n < 2048; n++) {
					data.writeFloat(0);
					data.writeFloat(0);
				}
				return;
			}
			
			// データが受け取れたら再生する
			bytes.endian = Endian.LITTLE_ENDIAN;
			bytes.position = 0;

			// 2048 回書くとうまく行く
			for (var i:uint = 0; i < 2048; i++) {
				// 左右それぞれ 16 bit ずつ受け取る
				var sample1:Number = bytes.readShort();
				var sample2:Number = bytes.readShort();
				
				// 16 bit int で受け取ったサウンドデータを Flash 形式の Number に変換する
				sample1 /= 32768;
				sample2 /= 32768;
				
				// 変換したデータを書き込む
				data.writeFloat(sample1);
				data.writeFloat(sample2);
			}
		}
		
		private var _process:Boolean = false;
		
		/**
		 * Flash から毎フレーム呼び出される.
		 * @param	e
		 */
		private function _onEnterFrame(e:Event):void {
			if (_fileReference.data == null) {
				return;
			}
			//if (_process) { return ;  }
			
			_process = true;
			
			// xpce の処理を 1 フレーム分進める
			nextFrame();
			
			// xpce から画面のデータを 1 フレーム分受け取る
			var bytes:ByteArray = getBytes();
			bytes.endian = Endian.LITTLE_ENDIAN;
			bytes.position = 0;
			_bitmapData.setPixels(_bitmapData.rect, bytes);
			
			// キー情報を Flash から受け取って xpce に渡す
			var down:int = 0;
			var up:int = 0;
			var left:int = 0;
			var right:int = 0;
			var start:int = 0;
			var select:int = 0;
			var b:int = 0;
			var a:int = 0;
			
			if (_keyInput.isKeyHeld(Keyboard.DOWN)) {
				down = 1;
			}
			if (_keyInput.isKeyHeld(Keyboard.UP)) {
				up = 1;
			}
			if (_keyInput.isKeyHeld(Keyboard.LEFT)) {
				left = 1;
			}
			if (_keyInput.isKeyHeld(Keyboard.RIGHT)) {
				right = 1;
			}
			if (_keyInput.isKeyHeld(Keyboard.ENTER)) {
				start = 1;
			}
			if (_keyInput.isKeyHeld(Keyboard.SHIFT)) {
				select = 1;
			}
			if (_keyInput.isKeyHeld(Keyboard.Z)) {
				b = 1;
			}
			if (_keyInput.isKeyHeld(Keyboard.X)) {
				a = 1;
			}
			setKeys(down, up, left, right, start, select, b, a);
			_keyInput.refresh();
			
			_process = false;
		}
		
	}
	
}