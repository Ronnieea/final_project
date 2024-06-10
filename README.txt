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
2.角色 (character): 定義遊戲中的角色，包括親和力、頭像、名稱和立繪。
3.物品 (item): 定義遊戲中的物品，包括圖標和名稱。
4.玩家 (player): 定義玩家的屬性，包括背包中的物品和角色角色。
5.事件 (event): 定義觸發的事件，事件包含對話和場景轉換。
6.對話 (dialogue): 定義遊戲中的對話，包括角色、文本和選項，每個選項可以引導到下一個對話或觸發事件。

