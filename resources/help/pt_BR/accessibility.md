# Leitores de tela, alto contraste e texto alternativo
<!-- keywords: acessibilidade, leitor de tela, narrador, cego, baixa visão, alto contraste, modo escuro, tema escuro, contorno de foco, texto alternativo, descrição de imagem, zoom, ampliar, somente teclado -->

O JeffPub foi criado para que pessoas que usam leitores de tela, o teclado ou cores de alto contraste também possam usá-lo.

## Leitores de tela

Um **leitor de tela** é um programa que lê em voz alta o conteúdo da tela. O JeffPub funciona com leitores de tela como o Narrador, no Windows. Todo controle tem um nome e uma descrição. Isso inclui as guias, os botões, as galerias e os botões de cor da faixa de opções, a lista de páginas, a página, os painéis de tarefas e a barra de status.

Os objetos na página também são lidos. Na página, pressione **Tab** para selecionar o próximo objeto: o leitor de tela diz que tipo de objeto é e o que contém, como "Caixa de texto: Promoção de primavera" ou "Imagem: um celeiro vermelho", depois onde ele está e qual é o seu tamanho. Uma imagem sem texto alternativo é lida como "Imagem, sem texto alternativo", para que você saiba quais ainda precisam dele. Pressione **Enter** para digitar em uma caixa de texto; o leitor de tela pode então ler o texto, a linha ou a palavra em que está o cursor e o que você selecionar.

- A lista de páginas lê cada página como "Página 1", "Página 2" e assim por diante. Se você deu um nome a uma página, o nome vem em seguida, como em "Página 3, Capa".
- A área da página é chamada de "Página". Sua descrição informa as teclas a usar: Tab seleciona o próximo objeto, as setas o movem, Enter edita o seu texto e Delete o remove.
- As guias são lidas como guias da faixa de opções. As guias que aparecem ao selecionar algo também informam o grupo a que pertencem, como Ferramentas de Imagem.

## Usar o teclado

Você pode acessar a faixa de opções, a lista de páginas e os objetos na página somente com o teclado. Pressione F6 para alternar entre a faixa de opções, a lista de páginas e a página. Pressione Alt para ver as Dicas de Tecla. Um contorno colorido mostra qual controle tem o foco. Consulte [Usar o JeffPub com o teclado](keyboard).

## Alterar as cores

- **Escuro ou claro**: clique em **Arquivo > Opções** e, na guia **Geral**, defina **Tema** como **Claro**, **Escuro** ou **Usar configuração do sistema**. A mudança ocorre imediatamente.
- **Alto contraste**: no Windows, ao ativar um tema de alto contraste, o JeffPub o acompanha. Ele usa as cores escolhidas pelo Windows e muda conforme você ativa ou desativa o tema.

## Tornar as imagens mais fáceis de entender

**Texto alternativo** (texto alternativo) é uma descrição curta de uma imagem ou de outro objeto. Os leitores de tela o leem em voz alta. O JeffPub também o inclui nas páginas da Web e nos e-books que você salvar.

1. Clique com o botão direito na imagem ou no objeto.
2. Clique em **Formatar Objeto…**.
3. Clique na guia **Texto Alternativo**.
4. Digite uma descrição na caixa **Texto alternativo**.
5. Clique em **OK**.

Descreva em poucas palavras o que a imagem mostra, como "Duas crianças plantando uma árvore". Os ícones e os códigos de barras que você inserir recebem texto alternativo automaticamente.

## Verificar a página

Clique em **Revisão > Verificar Acessibilidade** para listar o que pode ser difícil de usar para algumas pessoas. Além de imagens e formas sem texto alternativo, o verificador encontra mais cinco problemas.

- **Texto difícil de ler.** O texto precisa de *contraste* suficiente com o que está atrás dele: o preenchimento da sua caixa ou a cor da página, quando a caixa não tem preenchimento. O verificador calcula o contraste como uma razão. Uma razão de 4,5 para 1 basta para texto comum, e 3 para 1 basta para texto grande (18 pontos ou mais, ou 14 pontos ou mais em negrito). Por exemplo, cinza #777777 sobre branco tem razão 4,48 para 1, um pouco abaixo do mínimo, e #767676 tem 4,54 para 1. Escolha uma cor de texto mais escura ou um fundo mais claro. Quando o texto está sobre uma imagem, o verificador não consegue calcular o contraste; por isso, acrescenta uma dica para você conferir o contraste a olho.
- **Objetos lidos em outra ordem.** Um leitor de tela e um PDF com marcações leem os objetos de uma página da parte de trás da pilha para a da frente. Se essa ordem não for de cima para baixo e depois da esquerda para a direita, o verificador lista como dicas os objetos que estão fora de lugar. Clique em **Ordem de Leitura** para ver os objetos na ordem em que são lidos. Selecione um e clique em **Mover para Antes** ou **Mover para Depois**, ou arraste-o. Cada movimento é uma etapa de Desfazer e também altera qual objeto fica na frente.
- **Tabela sem linha de cabeçalho.** Um leitor de tela usa a primeira linha de uma tabela para dar nome às colunas. Uma tabela cuja primeira linha não está marcada como linha de cabeçalho é listada como aviso. Clique no problema e, depois, em **Usar a primeira linha como cabeçalho** para ativar a linha de cabeçalho. É uma etapa de Desfazer.
- **Tabela com células mescladas.** Um leitor de tela pode perder a posição em uma tabela quando há células mescladas; por isso, uma tabela com célula mesclada é listada como aviso. Mantenha cada célula de uma tabela inteira: separe as células mescladas ou use duas tabelas.
- **Link que não informa o destino.** Um leitor de tela pode ler os links de uma página como uma lista; por isso, as palavras de cada link devem indicar a página que ele abre. Um link cujo texto é o próprio endereço, é apenas "aqui", "clique aqui", "leia mais" ou uma expressão semelhante, ou está vazio, é listado como aviso. Altere o texto, como em "Nosso relatório anual" em vez de "clique aqui". Os primeiros 50 são listados um a um, e o restante é contado em uma única linha.

## Ampliar a página

Amplie o zoom para ler textos pequenos. Use os controles de **Zoom** na barra de status, mantenha Ctrl pressionada e gire a roda do mouse, ou escolha um tamanho na caixa **Zoom** da guia **Exibir**. O controle deslizante de zoom vai de cerca de 10% a 1600%.

## Consulte também

- [Usar o JeffPub com o teclado](keyboard)
- [Adicionar e ajustar imagens](pictures)
- [Alterar as configurações do JeffPub](options)
- [Salvar como imagem, página da Web ou e-book](export)
