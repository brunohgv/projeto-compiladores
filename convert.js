const fs = require('fs')

const string = fs.readFileSync('./input_string.txt', 'utf-8')

let output = ""

output += "{ "

output += string.split('').map(el => {
  switch(el) {
    case '(':
      return 'AP'
    case ')':
      return 'FP'
    default:
      return el
  }
}).join(', ').replace('(', 'AP').replace(')', 'FP')

output += " };"


fs.writeFileSync('./converted.txt', output)