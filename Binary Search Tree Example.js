class binTree{ //super simple node with 2 children class
    constructor(num){
        this.left = null
        this.right = null
        this.num = num
    }
    
}

function insertItem(tree,num){ //simple algorithm which just inserts a node based on the rules mentioned
    let inserting = true
    let currentNode = tree
    if(tree.num == null){
        currentNode.num = num
        return tree
    }
    while(inserting){
        if(currentNode.num <=num){
            if(currentNode.left== null){
                currentNode.left = new binTree(num)
                inserting = false
            }else{
                currentNode = currentNode.left
            }
        }else{
            if(currentNode.right== null){
                currentNode.right = new binTree(num)
                inserting = false
            }else{
                currentNode = currentNode.right
            }
        }
    }
    return tree
}

let tree = new binTree() //inserting all the nodes to form a tree as shown in my diagram
insertItem(tree,10)
insertItem(tree,8)
insertItem(tree,12)
insertItem(tree,1)
insertItem(tree,9)
insertItem(tree,11)
insertItem(tree,20)

function outTree(tree,index=0){ //printing out the tree into plain text, example of how it might be stored in memory (just imagine line number as memory adress)
    console.log(tree)
    let out = ""
    if(tree==null||tree.num == null) return out
    out+=`\n[line ${index}]\n`
    out+=`value: ${tree.num}\n`
    out+=`right: ${tree.right==null?"null":`line ${index+5}`}\n`
    out+=`left: ${tree.left==null?"null":`line ${index+10}`}\n`
    out+=outTree(tree.right,index+out.split("\n").length-1)
    out+=outTree(tree.left,index+out.split("\n").length-1)
    return out
}
console.log(outTree(tree))
