劇本檔格式說明：
1. 場景 (scene)
每個場景有唯一的標識符，每個場景包含背景圖片和名稱。
[scene.<scene_id>]
background = "<背景圖片的路徑>"
name = "<場景名稱>"

2. 角色 (character)
每個角色有唯一的標識符，包含親和力、頭像、名字和立繪。
[character.<character_id>]
affinity = <親和力數值>
avatar = "<頭像圖片的路徑>"
name = "<角色名稱>"
tachie = "<立繪圖片的路徑>"

3. 物品 (item)
每個物品有唯一的標識符，包含圖標和名稱。
[item.<item_id>]
icon = "<圖標圖片的路徑>"
name = "<物品名稱>"

4. 玩家 (player)
玩家包含背包中的物品和角色角色。
[player]
inventory = ["<物品_id>", "<物品_id>", ...]
role = "<角色角色>"

5. 事件 (event)
每個事件有唯一的標識符，包含對應的對話和場景。
[event.<event_id>]
dialogue = "<對話_id>"
scene = "<場景_id>"

6. 對話 (dialogue)
每段對話有唯一的標識符，包含角色、文本和選項。
[dialogue.<dialogue_id>]
character = "<角色_id>"
text = "<對話文本>"

[[dialogue.<dialogue_id>.options]]
effect = <效果數值>
next = "<下一個對話_id>" 或 event = "<事件_id>"
text = "<選項文本>"


示例
以下是一個完整的示例，展示了不同部分的定義：

場景
[scene.market]
background = "/example_game/assets/market.bmp"
name = "熱鬧市場"

角色
[character.knight]
affinity = 50
avatar = "/example_game/assets/knight_avatar.bmp"
name = "騎士阿勇"
tachie = "/example_game/assets/knight_tachie.bmp"

物品
[item.sword]
icon = "/example_game/assets/sword_icon.bmp"
name = "魔法劍"

玩家
[player]
inventory = ["sword", "potion", "scroll"]
role = "knight"

事件
[event.start]
dialogue = "intro"
scene = "market"

對話
[dialogue.intro]
character = "knight"
text = "今天是個充滿希望的日子，我需要準備好迎接挑戰。"

[[dialogue.intro.options]]
effect = "+10"
next = "meet_merchant"
text = "前往市場準備裝備。"

說明：
1.場景 (scene): 定義遊戲中的不同場景，包括背景圖片和名稱。
2.角色 (character): 定義遊戲中的角色，包括心情值、頭像、名稱和立繪。
3.物品 (item): 定義遊戲中的物品，包括圖標和名稱。
4.玩家 (player): 定義玩家的屬性，包括背包中的物品和角色角色。
5.事件 (event): 定義觸發的事件，事件包含對話和場景轉換。
6.對話 (dialogue): 定義遊戲中的對話，包括角色、文本和選項，每個選項可以引導到下一個對話或觸發事件。

遊戲方式：

1.基本架構
這是一個互動式小說引擎，您有兩種方式啟用這個引擎並遊玩遊戲：

第一種 -l -L ［output.toml], the name cannot be “script.toml”, and defult “output.toml”.

這個get option意指使用LLM（大型語言模型）自動生成圖片、臺詞、掉落物等遊戲素材，並且需要簡單的描述您的劇情和角色等需求，隨後LLM會為您成一個符合要求格式的劇本檔，並可使用引擎開始遊玩。

第二種 -s -S ,use your own script to play with this game engine,and defult “script.toml”.

這個get option意指使用預設或您準備的劇本檔、圖片、掉落物等，使用遊戲引擎遊玩遊戲。

遊戲畫面及操作簡介：

該event中圖片資訊指向的bmp圖片，會被印出作為背景圖片。

dialogue 的text會被印在下面的白框中作為角色對話。若下方無其他選項，則按tab鍵跳到下一個對話。

dialogue.option 的text，會被作為選項放在下面，供玩家選擇。玩家只需按下ABC選項的鍵盤即可選擇該對話，並跳到下一個畫面。

-t代表開啟背包以及心情直統計頁面，打開後可以看到當前蒐集到的物品，再一下可以收回畫面。

畫面右上角會顯示心情指數

特別要求

sudo apt update
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libopenal-dev libalut-dev libsndfile1-dev libcjson-dev

1. SDL2 和相關的 SDL2_image 和 SDL2_ttf
2. OpenAL 和相關的 ALUT
3. libsndfile
4.  cJSON
