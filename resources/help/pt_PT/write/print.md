# Imprimir
<!-- keywords: páginas por folha, várias páginas numa folha, ajustar ao tamanho do papel, reduzir, ajustar ao papel, páginas ímpares, páginas pares, inverter, última página primeiro, frente e verso, seleção, secção, p2s3, zoom, várias páginas, frente e verso manualmente, duplex, virar a pilha, imprimir, impressora, impressão, cópias, agrupadas, intervalo de páginas, páginas, página atual, pré-visualizar, papel, orientação, horizontal, vertical, pdf, guardar como pdf, ctrl+p, folha, secção -->

Prima **Ctrl+P**, ou clique em **Ficheiro > Imprimir**, para abrir a página Imprimir. As opções estão à esquerda. À direita está uma imagem de cada página tal como será impressa. Use as setas por baixo dela para ver as outras páginas. **Mostrar**, acima dela, escolhe uma, duas ou quatro páginas de cada vez. **Zoom** aumenta as páginas para além do tamanho que cabe no ecrã. Desloque-se para ver o resto.

## Escolher como imprimir

- **Impressora** lista as impressoras que o computador conhece. **Guardar como PDF** cria um ficheiro PDF em vez de papel. O JeffWrite pede um nome quando clica em **Imprimir**.
- **Cópias** indica quantas cópias imprimir.
- **Agrupadas** é importante quando imprime mais do que uma cópia. Com esta opção ativada, as páginas saem por esta ordem: 1, 2, 3, e depois 1, 2, 3 outra vez. Com a opção desativada, saem como 1, 1, 2, 2, 3, 3.
- **Todas as páginas**, **Página atual**, **Seleção** ou **Páginas** escolhe o que imprimir. **Seleção** imprime apenas o que selecionou no documento, com o mesmo papel, estilos, cabeçalhos e rodapés. Fica a cinzento quando não há nada selecionado.
- Para **Páginas**, também pode indicar uma página de uma secção. Escreva **p2s3** para a segunda página da secção 3, ou **s3** para toda a secção 3. Uma secção é uma parte do documento configurada à parte no separador **Esquema**. Pode misturá-las: 1-2, s3, p1s4-p2s4.
- Para **Páginas**, escreva números e intervalos de páginas separados por vírgulas, como 1-3, 5. Um intervalo pode ir no sentido inverso (5-3). Um intervalo sem fim vai até à última página (8-) ou começa na primeira (-2).
- A lista por baixo dos números de página pode limitar a impressão a **Apenas páginas ímpares** ou **Apenas páginas pares**. Conta as páginas por ordem, a partir da primeira página do documento: a primeira página é a página 1 (ímpar), a segunda é a página 2 (par), e assim sucessivamente. Não usa os números impressos nas páginas; por isso, um documento cuja numeração recomeça, ou começa em 5, continua a ter a primeira página contada como ímpar. O limite aplica-se às páginas escolhidas pelo intervalo.
- **Última página primeiro** imprime as páginas por ordem inversa, para que a pilha saia por ordem. Em conjunto com as opções de páginas ímpares e pares, imprime os dois lados de uma folha manualmente: imprima as páginas ímpares, vire a pilha e depois imprima as páginas pares, com **Última página primeiro** ativada ou desativada, conforme a sua impressora.
- A lista **Páginas por folha** coloca 2, 4, 6, 9 ou 16 páginas em cada folha. O JeffWrite reduz cada página para caber no seu lugar e desenha à volta uma linha fina. As páginas são dispostas em linhas, a partir do canto superior esquerdo. Uma folha com 2 ou 6 páginas é rodada para o lado, para que as páginas não fiquem comprimidas. A pequena imagem na página não mostra isto: mostra cada página inteira.
- A lista **Escalar para** faz com que cada página caiba num tamanho de papel: Carta, Ofício, A4, A5 ou A3. A página mantém a sua forma e fica centrada na folha. Use-a quando o papel na impressora não for o tamanho para o qual o documento foi criado.
- **Frente e verso manualmente** destina-se a uma impressora que não consegue imprimir nos dois lados. O JeffWrite imprime primeiro as páginas ímpares. Depois, uma mensagem pede-lhe para virar a pilha e voltar a colocá-la na impressora. Ao clicar em **OK**, imprime as páginas pares, da última para a primeira, para que fiquem no verso das páginas ímpares. Não é usada quando imprime para um ficheiro PDF, porque um ficheiro não tem lados.
- **Papel** indica em que papel é impressa a página da pré-visualização.

As páginas são impressas pela ordem em que as escreve, uma vez cada. Um número que não seja uma página do documento é ignorado. Se nenhum dos números escritos for uma página, a caixa fica com uma margem vermelha e o botão **Imprimir** fica inativo.

Clique em **Imprimir** para enviar as páginas. A página Imprimir fecha-se quando a impressora as tiver recebido.

## Os intervalos de páginas contam posições, não números impressos

Os números que escreve em **Páginas** são posições no documento: 1 é a primeira página, 2 é a segunda, e assim sucessivamente, independentemente do número que cada página mostra. Se uma secção reiniciar a numeração, como a página 1 de um novo capítulo, ou começar com outro número, o número mostrado por uma página pode ser diferente da sua posição. Escreva a posição. A linha por baixo da pré-visualização, como **Página 5 de 12**, indica a posição da página que está a ver, para que possa descobrir o número a escrever.

## O papel vem do documento

Cada secção de um documento tem o seu próprio tamanho de papel e a sua própria orientação, definidos no separador **Esquema**. O JeffWrite imprime cada página no papel da sua secção e roda para o lado uma página na horizontal. Se as páginas escolhidas usarem mais do que um tipo de papel, a linha **Papel** di-lo. Coloque o papel de que cada página precisa.

## O papel e o PDF têm o mesmo aspeto

O JeffWrite desenha uma página da mesma forma para a impressora e para um ficheiro PDF: as mesmas linhas, as mesmas quebras e os mesmos tipos de letra. As linhas vermelhas da ortografia e as marcas de parágrafo nunca são impressas.

Para criar um PDF sem usar a página Imprimir, veja [Abrir, guardar, imprimir e exportar](open-save).

## Ver também

- [Verificar a ortografia](spelling)
- [Abrir, guardar, imprimir e exportar](open-save)
