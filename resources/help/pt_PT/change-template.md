# Alterar o modelo de uma publicação
<!-- keywords: alterar modelo, mudar modelo, aplicar modelo, trocar modelo, outra estrutura, nova estrutura, outro design, novo design, mudar aspeto, conteúdo adicional, texto restante, imagens restantes, marcador de posição, funções, criar nova publicação, mover texto e imagens -->

**Alterar Modelo** dá à publicação que tem aberta o aspeto de uma estrutura diferente e move o seu texto e as suas imagens para essa estrutura. Não precisa de voltar a escrever nada. Tudo é feito num único passo, que pode anular.

## Alterar o modelo

1. Clique em **Estrutura da Página > Modelo > Alterar Modelo**. A vista Ficheiro abre numa galeria de estruturas, com o título **Alterar Modelo**.
2. Escolha uma estrutura tal como faz para uma nova publicação: escolha um grupo à esquerda ou escreva na caixa **Procurar modelos** e, em seguida, clique numa estrutura. Consulte [iniciar uma nova publicação](create).
3. Se quiser, utilize o painel à direita. **Esquema de cores** e **Esquema de tipos de letra** começam em **(manter atual)**, o que mantém as cores e os tipos de letra que tem. Escolha aí um esquema apenas se quiser que a nova estrutura os altere. **Informações comerciais** escolhe o conjunto de dados de contacto que a estrutura preenche. **Incluir logótipo** e **Incluir endereço postal** só aparecem nas estruturas que os utilizam.
4. Clique em **Alterar Modelo**. Também pode fazer duplo clique na estrutura.
5. Escolha onde fica a nova estrutura e clique em **OK**:
   - **Aplicar o modelo à publicação atual** altera a publicação que tem aberta.
   - **Criar uma nova publicação com o meu texto e gráficos** abre uma segunda janela com o resultado. A publicação de partida não é alterada.

A galeria mostra apenas estruturas. Para alterar o tamanho do papel de uma publicação sem alterar a sua estrutura, utilize [configurar página](page-setup).

## Para onde vão o texto e as imagens

Um **artigo** é o texto de uma caixa de texto ou de uma cadeia de caixas de texto ligadas. Cada caixa de texto de uma estrutura tem uma **função**, que indica para que serve a caixa. As funções são:

- Título (o cabeçalho principal) e subtítulo
- Título de secção, para letra grande mais adiante na publicação
- Corpo do texto
- Data, que também contém um local ou uma hora
- Endereço e contactos: a morada, o telefone, o endereço de correio eletrónico e o endereço Web
- Organização, que é o nome e o slogan da sua organização
- Pessoa, que é o nome e o cargo de uma pessoa
- Legenda e citação em destaque
- Endereço postal, o bloco que a impressão em série preenche
- Texto curto, para pequenas etiquetas

As estruturas incluídas no JeffPub já têm estas funções. Para uma caixa que desenhou ou que vem de um ficheiro .pub, o JeffPub deduz a função a partir do texto (uma data, um número de telefone), do estilo (Legenda ou Citação) e do tamanho.

Cada um dos seus artigos vai para a caixa da nova estrutura com a mesma função. Por exemplo, o artigo que escreveu no corpo de um panfleto vai para a caixa de corpo de um boletim informativo, e o nome da sua organização vai para o cabeçalho. Se a nova estrutura tiver várias caixas com a mesma função, os seus artigos preenchem-nas pela ordem de leitura: página a página, de cima para baixo, da esquerda para a direita.

O artigo mantém as palavras, bem como o negrito, o itálico, as ligações e os campos. Assume o tipo de letra, a cor e o espaçamento da nova estrutura, para ficar com o mesmo aspeto do resto da estrutura. O título de uma estrutura que utiliza Texto Artístico fica com o primeiro parágrafo do seu artigo de título. Os parágrafos seguintes vão para Conteúdo Adicional.

As imagens vão para as **imagens de marcador de posição** da estrutura, que são as imagens provisórias que uma estrutura mostra até adicionar as suas. A primeira imagem ocupa o primeiro marcador de posição pela ordem de leitura, a imagem seguinte ocupa o seguinte, e assim sucessivamente. Cada imagem é recortada para preencher a moldura e mantém a sua luminosidade, contraste e recoloração. Os marcadores de posição para os quais não tem imagem mantêm a imagem provisória da estrutura.

Algumas coisas ficam como estão:

- O texto que nunca substituiu é o texto de exemplo da estrutura antiga. Não é transferido, e a nova estrutura mostra aí o seu próprio texto de exemplo.
- O tamanho da página, as margens, as páginas e as páginas modelo vêm da nova estrutura. Uma publicação com mais páginas do que a estrutura fica com as páginas da estrutura, e o resto do conteúdo vai para Conteúdo Adicional. O JeffPub não adiciona páginas para ele.
- As tabelas, formas, linhas, Texto Artístico e grupos de objetos criados pela estrutura antiga, e que não alterou, ficam com ela. Tudo o que desenhou e tudo o que alterou (uma tabela com uma célula em que escreveu, uma forma cujo texto editou, uma linha que moveu) vai inteiro para Conteúdo Adicional.
- As informações comerciais, as propriedades do documento, os estilos, a lista da impressão em série e os objetos na área de rascunho ficam com a publicação.

## Conteúdo Adicional

Nada do que criou é deitado fora. O conteúdo que não tem lugar na nova estrutura fica à espera no painel **Conteúdo Adicional**, que abre ao lado da página quando a alteração termina. O painel lista cada artigo, imagem, tabela, forma, linha e grupo que sobrou.

- Clique num item e, em seguida, clique em **Colocar na Página** para o colocar na página que está a ver. Um artigo torna-se uma caixa de texto à medida do texto, uma tabela ou um grupo volta inteiro, e uma imagem ou objeto demasiado grande é reduzido para caber na página. Também pode fazer duplo clique num item ou arrastá-lo para a página.
- Clique num item e, em seguida, clique em **Descartar** para o remover definitivamente. (Pode sempre anular.)

O painel fica com a publicação até a fechar e é guardado no ficheiro da publicação. (Um ficheiro .pub não tem lugar para ele, por isso, ao guardar como .pub, o painel não é incluído.) Para o abrir novamente, clique em **Estrutura da Página > Modelo > Conteúdo Adicional**. O botão fica esbatido quando não sobra nada.

## Anular

**Anular** (Ctrl+Z) repõe a estrutura antiga e volta a retirar o que sobrou de Conteúdo Adicional. Colocar ou descartar um item é um passo à parte.

## Relacionados

- [Iniciar uma nova publicação](create)
- [Esquemas de cores e de tipos de letra](color-schemes)
- [Tamanho da página, margens e esquema](page-setup)
- [O separador Estrutura da Página](tab-page-design)
