# Recortar, copiar e colar

<!-- keywords: recortar, copiar, colar, área de transferência, mover, duplicar, especial, valores, fórmulas, formatos, transpor, ignorar vazios, somar, subtrair, multiplicar, dividir, operação, mesclada, texto, guia, html, página da web, outro programa, Esc, formigas em marcha, borda, larguras de coluna, anotações, comentários, formatos de número, todas exceto bordas, imagem, bitmap, copiar como imagem, imagem impressa, como impresso, colar link, hiperlink, importação de texto, texto delimitado, soltar texto, vírgula, ponto e vírgula, csv, separador, definir colagem padrão, colagem padrão, largura fixa -->

**Copiar** (**{key:Copy}**) coloca uma cópia das células selecionadas na área de transferência, um espaço que o computador guarda para uma coisa de cada vez. **Colar** (**{key:Paste}**) coloca o que está na área de transferência nas células, começando pela célula selecionada. **Recortar** (**{key:Cut}**) funciona como copiar, mas, ao colar, as células são movidas e o local antigo fica vazio.

Uma borda tracejada em movimento mostra as células que foram copiadas ou recortadas. Pressione **Esc** para retirar a borda; então as células não estão mais prontas para colar. As células recortadas também são liberadas quando você faz qualquer outra alteração antes de colá-las (digitar, inserir, desfazer): recorte-as de novo se ainda quiser movê-las.

## Colar dentro do JeffCrunch

Tudo é colado: os valores, as fórmulas, os formatos e as células mescladas. Uma fórmula continua apontando para os mesmos lugares *em relação à sua posição*. Exemplo: copie **=A1*2** de B1 e cole em B5, e ela passa a ser **=A5*2**. Uma parte com cifrão, como **$A$1**, permanece como está. Se você selecionar um bloco que seja um número inteiro de vezes o tamanho do que foi copiado, a cópia se repete para preenchê-lo: copie uma célula, selecione vinte células, cole, e todas as vinte recebem o conteúdo.

Um número colado que seja largo demais para uma coluna que ainda tem a largura inicial alarga a coluna.

## Recortar move células

Quando você recorta células e as cola, elas realmente são movidas. Toda fórmula que apontava para elas passa a apontar para o novo local. Exemplo: B1 contém **=A1+A2**. Recorte A1:A2 e cole em E4, e B1 passa a ser **=E4+E5**. O que estava nas células sobre as quais você colou é perdido (**Desfazer** o traz de volta), e uma fórmula que lia uma dessas células mostra **#REF!**, porque a célula que ela lia não existe mais. Se as células recortadas forem apenas parte de uma célula mesclada, ou se você colar sobre apenas parte de uma, nada é movido: selecione a célula mesclada inteira. Ao recortar células para outra planilha, as fórmulas continuam lendo as células que liam antes (a planilha é indicada). A área de transferência é esvaziada após um recorte e uma colagem.

## Colar Especial

**Ctrl+Alt+V**, ou a seta abaixo de **Colar** na guia Página Inicial, permite escolher o que colar:

- **Tudo**, **Fórmulas**, **Valores** ou **Formatos**. *Valores* transforma uma fórmula no número ou texto que ela mostra.
- **Tudo exceto bordas**: tudo, mas as células mantêm as bordas que tinham.
- **Fórmulas e formatos de número**, ou **Valores e formatos de número**: as fórmulas (ou os valores) vêm com a forma como os números são mostrados, como porcentagem ou data, mas não as cores, fontes ou bordas. As células mantêm a aparência que tinham.
- **Larguras de coluna**: as colunas em que você cola recebem as larguras das colunas copiadas.
- **Comentários e anotações**: as anotações das células copiadas são colocadas nas células em que você cola. Nenhum valor de célula muda.
- Uma **operação**: **Somar**, **Subtrair**, **Multiplicar** ou **Dividir**. Os números copiados são combinados com os números que já estão nas células. Exemplo: as células contêm 10 e 20, você copiou 5, e escolheu *Multiplicar*: elas passam a ser 50 e 100.
- **Ignorar vazias**: células vazias na cópia deixam as células de destino como estavam.
- **Transpor**: linhas viram colunas e colunas viram linhas.

## O botão Opções de Colagem

Logo depois de colar células copiadas no JeffCrunch, um pequeno botão **Opções de Colagem** aparece ao lado da última célula colada. Clique nele para escolher outra forma de colocar as mesmas células: **Manter formatação da origem** (tudo), **Valores**, **Fórmulas**, **Formatação**, **Sem bordas**, **Fórmulas e formatação de número**, **Valores e formatação de número**, **Manter larguras de coluna da origem**, ou **Transpor**. O JeffCrunch desfaz a colagem e a refaz desse modo. A alteração é uma única etapa: um único **Desfazer** desfaz toda a colagem, por mais vezes que você tenha mudado de ideia. Exemplo: copie A1:B2, onde B1 é **=A1*2**, e cole em F5. Escolha **Valores** e G5 passa a conter o número 2 em vez de **=F5*2**; escolha **Manter formatação da origem** e a fórmula volta.

Pelo teclado, pressione a tecla **Ctrl** (a tecla **Control** em um Mac) depois de colar para abrir o menu do botão e use as setas e **Enter** para escolher. O botão desaparece quando você altera uma célula ou qualquer outra coisa na pasta de trabalho, copia outra coisa, pressiona **Esc**, ou rola a última célula colada para fora da tela (ele volta quando você rola de volta). Ele não é oferecido após um recorte e uma colagem, nem para texto de outro programa.

## Colar Link, Colar como hiperlink e Importação de Texto ao colar

Mais três opções estão na seta abaixo de **Colar**. Elas funcionam com células copiadas na mesma pasta de trabalho, exceto a última.

- **Colar link** coloca uma fórmula em cada célula que aponta para a célula copiada. Exemplo: copie A1:B2, selecione D4 e use **Colar link**: D4 passa a ser **=Planilha1!$A$1**. Quando A1 muda, D4 muda junto. Uma célula copiada que está vazia mostra 0.
- **Colar como hiperlink** coloca em cada célula as mesmas palavras que a célula copiada mostra, e transforma cada uma em um link para a célula copiada. Mantenha **Ctrl** pressionada e clique em uma delas para ir até essa célula. Funciona com até 2.000 células de cada vez.
- **Importação de Texto ao colar** serve para texto vindo de outro programa. Ela abre a mesma janela usada para abrir um arquivo de texto: você informa o que separa as partes (vírgula, tabulação, espaço, ponto e vírgula, barra vertical ou outro caractere), o que envolve uma parte entre aspas, a primeira linha a usar e como ler cada coluna (como digitado, como texto, como data, ou ignorada). Alguns textos alinham as partes em colunas de mesma largura, com espaços entre elas. Marque **As colunas têm largura fixa** e informe onde cada coluna começa: cada linha é cortada nesses pontos, e os espaços em volta de cada parte são removidos. Escolha *texto* para uma coluna de códigos como 007, para que os zeros sejam mantidos. As partes são colocadas nas células, começando pela célula selecionada, como uma única etapa que pode ser desfeita.

## Texto com vírgulas, ponto e vírgula ou colunas

Quando o texto que você cola de outro programa é claramente uma tabela, o JeffCrunch abre a mesma janela usada para abrir um arquivo de texto, para que você possa conferir como ele foi separado antes que algo seja colocado nas células. Isso acontece apenas com um **Colar** comum (**{key:Paste}**), nunca com **Colar Especial** nem com uma de suas opções. Acontece quando o texto tem pelo menos duas linhas e nenhuma tabulação, e ou todas as linhas têm o mesmo número de partes, pelo menos três, separadas por vírgulas, ponto e vírgula ou barras, com algo em cada coluna (uma parte entre aspas pode conter vírgula); ou as partes se alinham em colunas de mesma largura, com os mesmos intervalos de espaços em todas as linhas (pelo menos três linhas e pelo menos dois intervalos). Números com separador de milhar, como 1,234, e linhas de código ou listas que terminam em vírgula ou ponto e vírgula não são tratados como tabela. A janela começa com a forma como o texto foi identificado. Se os números tiverem vírgulas e as partes forem separadas por ponto e vírgula, a vírgula é o separador decimal. Clique em **OK** para colocar as partes nas células, ou em **Cancelar** para não colocar nada.

Exemplo: copie estas linhas de um e-mail.

    nome,qtd,preço
    Maçã,12,1.50
    Pera,3,0.25

Cole-as com uma terceira coluna, como um preço, e a janela mostra três colunas, com **Vírgula** marcada. Uma frase em uma só linha, linhas com quantidades diferentes de vírgulas ou apenas duas partes por linha são coladas como palavras, sem janela. Texto com tabulações vai direto para as células, como sempre foi. Para abrir a janela para qualquer texto, escolha **Importação de Texto ao colar** (acima).

O mesmo acontece quando você arrasta texto de outro programa e o solta na grade: ele é inserido na célula selecionada. Uma soltura em qualquer outro lugar da janela não faz nada.

## Definir colagem padrão

**Definir colagem padrão** é a última opção na seta abaixo de **Colar**. Ela decide o que o botão **Colar** e **{key:Paste}** colocam quando você copiou células: tudo (no início), ou somente os valores, somente as fórmulas, somente a formatação, e assim por diante. Exemplo: escolha *Valores*, e uma fórmula copiada aparece como o número que mostra. A escolha fica guardada para a próxima vez. Ela não altera **Colar Especial** nem as opções do menu, e o texto de outros programas é colado como está. Escolha *Tudo* para voltar ao padrão.

A mesma escolha está na página **Edição** de **Opções**, em duas caixas: uma para células copiadas nesta pasta de trabalho e outra para células copiadas em outra pasta de trabalho. Até que você escolha uma para outra pasta de trabalho, ela segue a primeira.

## O menu Colar e as imagens

A seta abaixo de **Colar** lista as opções habituais e também **Sem bordas**, **Fórmulas e formatação de número**, **Valores e formatação de número**, **Manter larguras de coluna da origem** e **Comentários e anotações**. São as mesmas opções de Colar Especial, com um clique cada.

A seta ao lado de **Copiar** tem **Copiar como imagem** e **Copiar como bitmap**. Cada uma coloca na área de transferência uma imagem das células selecionadas, desenhada como você as vê, com as linhas de grade. Um bitmap tem o tamanho da tela. Uma imagem tem o dobro de pontos, por isso continua nítida quando ampliada. **Copiar como imagem impressa** desenha as células como o papel as mostraria: as linhas de grade só aparecem se a configuração de página as imprimir. Cole no JeffWrite ou no JeffDeck como imagem. Se as células não couberem em uma só imagem, selecione menos células.

## Entre programas

Ao copiar, o JeffCrunch também coloca as células na área de transferência como texto (células separadas por tabulações, uma linha por registro) e como tabela para páginas da web, para que outros programas possam usá-las. Quando você cola algo que não veio do JeffCrunch, como texto com tabulações ou uma tabela de uma página da web ou de outra planilha, cada parte vai para a sua própria célula e é lida como se você a tivesse digitado: números, datas e fórmulas que começam com sinal de igual. Copiar de outra pasta de trabalho aberta no JeffCrunch traz valores, fórmulas e formatos de número, mas não as outras formatações. Cole-os no JeffDeck ou no JeffPub, e eles aparecem como uma tabela que mantém as fontes, os sombreamentos, as bordas e os números como são mostrados (veja **Colar células como tabela** na Ajuda deles).

## O que não pode ser colado

O JeffCrunch avisa e não altera nada quando a colagem atingiria parte de uma **fórmula matricial**, a linha de nomes de colunas de uma **tabela**, células bloqueadas de uma **planilha protegida**, além do fim da planilha, ou mais de 100.000 células. Uma **fórmula matricial** inteira pode ser recortada e movida (selecione todas as suas células); parte dela, uma tabela de dados ou a linha de nomes de uma tabela não podem.
