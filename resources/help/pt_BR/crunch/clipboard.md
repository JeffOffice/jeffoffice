# Recortar, copiar e colar

<!-- keywords: recortar, copiar, colar, área de transferência, mover, duplicar, colar especial, valores, fórmulas, formatos, transpor, pular em branco, somar, subtrair, multiplicar, dividir, operação, mesclada, texto, tabulação, html, página da web, outro programa, Esc, formiguinha, borda, larguras de coluna, anotações, comentários, formatos de número, tudo exceto bordas, imagem, bitmap, copiar como imagem, imagem impressa, como impresso, colar link, hiperlink, importar texto, dividir texto, separador, definir colagem padrão, colagem padrão, largura fixa -->

**Copiar** (**{key:Copy}**) coloca uma cópia das células selecionadas na área de transferência, um espaço que o computador guarda para uma coisa de cada vez. **Colar** (**{key:Paste}**) coloca o que está na área de transferência nas células, a partir da célula selecionada. **Recortar** (**{key:Cut}**) funciona como copiar, mas, ao colar, as células são movidas e o local original fica vazio.

Uma borda tracejada em movimento mostra as células copiadas ou recortadas. Pressione **Esc** para remover a borda; as células então deixam de estar prontas para colar. As células recortadas também são liberadas se você fizer qualquer outra alteração antes de colá-las (digitar, inserir, desfazer): recorte-as novamente se ainda quiser movê-las.

## Colar dentro do JeffCrunch

Tudo vem junto: os valores, as fórmulas, os formatos e as células mescladas. Uma fórmula continua apontando para os mesmos lugares *em relação à sua posição*. Exemplo: copie **=A1*2** de B1 e cole em B5, e ela se torna **=A5*2**. Uma parte com cifrão, como **$A$1**, permanece como está. Se você selecionar um bloco que seja um número inteiro de vezes o tamanho do que foi copiado, a cópia se repete para preenchê-lo: copie uma célula, selecione vinte células, cole, e todas as vinte recebem o conteúdo.

Um número colado que seja largo demais para uma coluna que ainda tenha a largura inicial amplia a coluna.

## Recortar move células

Quando você recorta células e as cola, elas realmente são movidas. Toda fórmula que apontava para elas passa a apontar para elas no novo local. Exemplo: B1 contém **=A1+A2**. Recorte A1:A2 e cole em E4, e B1 se torna **=E4+E5**. O que estava nas células sobre as quais você colou é perdido (**Desfazer** o restaura), e uma fórmula que lia uma dessas células mostra **#REF!**, porque a célula que ela lia não existe mais. Se as células recortadas forem apenas parte de uma célula mesclada, ou se você colar sobre apenas parte de uma delas, nada é movido: selecione a célula mesclada inteira. Ao recortar células para outra planilha, as fórmulas continuam lendo as células que liam antes (indicando a planilha). A área de transferência é esvaziada após um recorte e uma colagem.

## Colar Especial

**Ctrl+Alt+V**, ou a seta abaixo de **Colar** na guia Página Inicial, permite escolher o que colar:

- **Tudo**, **Fórmulas**, **Valores** ou **Formatos**. *Valores* transforma uma fórmula no número ou texto que ela mostra.
- **Tudo Exceto Bordas**: tudo, mas as células mantêm as bordas que tinham.
- **Fórmulas e Formatos de Número** ou **Valores e Formatos de Número**: as fórmulas (ou os valores) vêm com a forma como os números são exibidos, como porcentagem ou data, mas não as cores, fontes ou bordas. As células mantêm a aparência que tinham.
- **Larguras de Coluna**: as colunas em que você colar recebem as larguras das colunas copiadas.
- **Comentários e Anotações**: as anotações das células copiadas são colocadas nas células em que você colar. Nenhum valor de célula muda.
- Uma **operação**: **Somar**, **Subtrair**, **Multiplicar** ou **Dividir**. Os números copiados são combinados com os números já existentes nas células. Exemplo: as células contêm 10 e 20, você copiou 5 e escolheu *Multiplicar*: elas passam a ter 50 e 100.
- **Pular em branco**: células vazias na cópia deixam as células de destino como estavam.
- **Transpor**: linhas viram colunas e colunas viram linhas.

## Colar Link, Colar como Hiperlink e Importar Texto ao Colar

Mais três opções estão na seta abaixo de **Colar**. Elas funcionam com células copiadas na mesma pasta de trabalho, exceto a última.

- **Colar Link** coloca em cada célula uma fórmula que aponta para a célula copiada. Exemplo: copie A1:B2, selecione D4 e use **Colar Link**: D4 se torna **=Planilha1!$A$1**. Quando A1 mudar, D4 muda junto. Uma célula copiada vazia mostra 0.
- **Colar como Hiperlink** coloca em cada célula as mesmas palavras que a célula copiada mostra, e transforma cada uma em um link para a célula copiada. Mantenha **Ctrl** pressionado e clique em uma delas para ir até essa célula. Funciona com até 2.000 células por vez.
- **Importar Texto ao Colar** serve para texto vindo de outro programa. Ele abre a mesma janela usada para abrir um arquivo de texto: você informa o que separa as partes (vírgula, tabulação, espaço, ponto e vírgula, barra vertical ou outro caractere), o que delimita uma parte (aspas), a primeira linha a usar e como ler cada coluna (como digitado, como texto, como data ou não importar). Alguns textos alinham suas partes em colunas de mesma largura, com espaços entre elas. Marque **As colunas têm largura fixa** e indique onde cada coluna começa: cada linha é cortada nesses pontos, e os espaços ao redor de cada parte são removidos. Escolha *texto* para uma coluna de códigos como 007, para que os zeros sejam mantidos. As partes são colocadas nas células, a partir da célula selecionada, como uma única etapa que você pode desfazer.

## Definir Colagem Padrão

**Definir Colagem Padrão** é a última opção na seta abaixo de **Colar**. Ela decide o que o botão **Colar** e **{key:Paste}** inserem quando você copiou células: tudo (no início), ou somente os valores, somente as fórmulas, somente a formatação, e assim por diante. Exemplo: escolha *Valores*, e uma fórmula copiada entra como o número que ela mostra. A escolha é mantida para a próxima vez. Ela não altera **Colar Especial** nem as opções do menu, e o texto de outros programas é colado como está. Escolha *Tudo* para voltar ao padrão.

## O menu Colar e as imagens

A seta abaixo de **Colar** lista as opções habituais e também **Sem Bordas**, **Fórmulas e Formatação de Número**, **Valores e Formatação de Número**, **Manter Larguras de Coluna de Origem** e **Comentários e Anotações**. São as mesmas opções de Colar Especial, com um clique cada.

A seta ao lado de **Copiar** tem **Copiar como Imagem** e **Copiar como Bitmap**. Cada uma coloca na área de transferência uma imagem das células selecionadas, desenhada como você as vê, com as linhas de grade. Um bitmap tem o tamanho da tela. Uma imagem tem o dobro de pontos, por isso continua nítida quando ampliada. **Copiar como Imagem Impressa** desenha as células como o papel as mostraria: as linhas de grade aparecem somente se a configuração da página as imprimir. Cole-a no JeffWrite ou no JeffDeck como imagem. Se as células não couberem em uma única imagem, selecione menos células.

## Entre programas

Ao copiar, o JeffCrunch também coloca as células na área de transferência como texto (células separadas por tabulações, uma linha por registro) e como tabela para páginas da web, para que outros programas possam usá-las. Ao colar algo que não veio do JeffCrunch, como texto com tabulações ou uma tabela de uma página da web ou de outra planilha, cada parte vai para a sua própria célula e é lida como se você a tivesse digitado: números, datas e fórmulas que começam com sinal de igual. Copiar de outra pasta de trabalho aberta no JeffCrunch traz valores, fórmulas e formatos de número, mas não a outra formatação.

## O que não pode ser colado

O JeffCrunch avisa e não altera nada quando a colagem atingiria parte de uma **fórmula de matriz**, a linha dos nomes de colunas de uma **tabela**, células bloqueadas de uma **planilha protegida**, além do fim da planilha ou em mais de 100.000 células. Uma fórmula de matriz inteira pode ser recortada e movida (selecione todas as suas células); parte dela, uma tabela de dados ou a linha de nomes de uma tabela não podem.
