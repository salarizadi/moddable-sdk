<img src="https://github.com/salarizadi/moddable-sdk/assets/67143370/50f90d56-d0ec-4076-8d46-ac441539a7f6" width="250" height="250"></img>
<img src="https://github.com/salarizadi/moddable-sdk/assets/67143370/25cd7ca9-fc56-458d-a43f-4d7d574aac7f" width="250" height="250"></img>

## 4x3 keyboard example (ESP32):
```js
import Keypad from "keypad"

new Keypad({
    map    : [
        ["1", "2", "3"],
        ["4", "5", "6"],
        ["7", "8", "9"],
        ["*", "0", "#"]
    ],
    rows   : [12, 14, 17, 16], // Enter in order
    columns: [4, 13, 2] // Enter in order
}, key => {
    trace(`\nYou pressed the key (${key})`)
})
```

## Warning
- Be careful to enter all the keyboard pins in order because it will cause the keys to be incorrectly displayed and not recognized correctly.
