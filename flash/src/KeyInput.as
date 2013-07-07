package 
{
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.events.KeyboardEvent;
	import flash.events.MouseEvent;
	import flash.system.IME;

	/**
	 * キー入力処理用クラス.
	 */
	public class KeyInput extends Sprite
	{
		/**
		 * キー定数の最大値
		 * 数が多いほど処理時間がかかる。
		 */
		private const MAX_KEYCODE:uint = 256;
		
		/**
		 * キー入力状態保存用変数
		 */
		private var keys:Vector.<Boolean>;
		
		/**
		 * 1 フレーム前の
		 * キー入力状態保存用変数
		 */
		private var prevKeys:Vector.<Boolean>;
		
		/**
		 * コンストラクタ
		 */
		public function KeyInput():void {
			// キー入力状態保存用変数の初期化
			// キー定数の最大値が収まるくらい作っておけば問題ないかと思って
			// 256 にしています。
			keys = new Vector.<Boolean>();
			prevKeys = new Vector.<Boolean>();
			for (var i:uint = 0; i < MAX_KEYCODE; i++) {
				keys.push(false);
				prevKeys.push(false);
			}
			
			// イベントの登録
			// addChild(), removeChild() に応じて、
			// 自動的に必要なイベントハンドラを登録・削除する
			addEventListener(Event.ADDED_TO_STAGE, onAddedToStage);
			addEventListener(Event.REMOVED_FROM_STAGE, onRemoveFromStage);
		}
		
		/**
		 * キー入力情報を更新
		 * 必ず ENTER_FRAME イベントの最後に呼び出す
		 */
		public function refresh():void {
			// 1 フレーム前のキーを保存して、次のフレームの開始時に比較することで
			// 押し始め、押し終わりを検出する
			for (var i:uint = 0; i < MAX_KEYCODE; i++) {
				prevKeys[i] = keys[i];
			}
		}
		
		/**
		 * キーが押されっぱなしになっているかチェック
		 * @param	keycode	Keyboard クラスのキー定数
		 * @return	押されている時 True
		 */
		public function isKeyHeld(keycode:uint):Boolean {
			return keys[keycode];
		}
		
		/**
		 * キーが押された直後かチェック
		 * @param	keycode	Keyboard クラスのキー定数
		 * @return	押され始めて最初のフレームの時 True
		 */
		public function isKeyDown(keycode:uint):Boolean {
			if (keys[keycode] === true && prevKeys[keycode] === false) {
				return true;
			}
			return false;
		}
		
		/**
		 * キーが離された直後かチェック
		 * @param	keycode	Keyboard クラスのキー定数
		 * @return	離された最初のフレームの時 True
		 */
		public function isKeyUp(keycode:uint):Boolean {
			if (keys[keycode] === false && prevKeys[keycode] === true) {
				return true;
			}
			return false;
		}
		
		/**
		 * addChild() された時に呼ばれるイベント
		 * @param	event
		 */
		private function onAddedToStage(event:Event):void {
			trace("KeyInput.onAddedToStage");
			// ステージ上のイベントの登録
			// キー入力とマウス入力はステージに登録する
			stage.addEventListener(KeyboardEvent.KEY_DOWN, onKeyDown);
			stage.addEventListener(KeyboardEvent.KEY_UP, onKeyUp);
			stage.addEventListener(MouseEvent.MOUSE_DOWN, onMouseDown);
		}
		
		/**
		 * removeChild() された時に呼ばれるイベント
		 * @param	event
		 */
		private function onRemoveFromStage(event:Event):void {
			trace("KeyInput.onRemoveFromStage");
			
			// キー入力を全て OFF にする
			for (var i:uint = 0; i < MAX_KEYCODE; i++) {
				keys[i] = false;
				prevKeys[i] = false;
			}
			
			// 登録したイベントを解除する
			stage.removeEventListener(KeyboardEvent.KEY_DOWN, onKeyDown);
			stage.removeEventListener(KeyboardEvent.KEY_UP, onKeyUp);
			stage.removeEventListener(MouseEvent.MOUSE_DOWN, onMouseDown);
		}
		
		/**
		 * ステージ上でマウスが押された時のイベント
		 * @param	e
		 */
		private function onMouseDown(event:MouseEvent):void {
			// IME を無効にする。
			// 環境によっては IME 無効処理時に例外が発生するので、例外をスルーする
			try { IME.enabled = false; }
			catch (e:Error) { ; }
		}
		
		/**
		 * キーが押された時のイベント
		 * @param	event
		 */
		private function onKeyDown(event:KeyboardEvent):void {
			// 入力 ON
			keys[event.keyCode] = true;
		}
		
		/**
		 * キーが離された時のイベント
		 * @param	event
		 */
		private function onKeyUp(event:KeyboardEvent):void {
			// 入力 OFF
			keys[event.keyCode] = false;
		}
	}
}