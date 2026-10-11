# Alterar o modelo de uma publicação
<!-- keywords: alterar modelo, mudar modelo, trocar modelo, aplicar modelo, outro design, novo design, mudar o visual, conteúdo extra, texto que sobrou, imagens que sobraram, espaço reservado, funções, criar uma nova publicação, mover texto e imagens -->

**Alterar Modelo** dá à publicação aberta a aparência de outro design e move seu texto e suas imagens para ele. Você não precisa redigitar nada. Tudo acontece em uma única etapa, que pode ser desfeita.

## Alterar o modelo

1. Clique em **Design da Página > Modelo > Alterar Modelo**. O modo de exibição Arquivo é aberto em uma galeria de designs, com o título **Alterar Modelo**.
2. Escolha um design da mesma forma que faz para uma nova publicação: escolha um grupo à esquerda ou digite na caixa **Pesquisar modelos** e clique em um design. Consulte [iniciar uma nova publicação](create).
3. Se quiser, use o painel à direita. **Esquema de cores** e **Esquema de fontes** começam em **(manter atual)**, que mantém as cores e fontes que você já tem. Escolha um esquema ali somente se quiser que o novo design as altere. **Informações comerciais** escolhe o conjunto de dados de contato que o design preenche. **Incluir logotipo** e **Incluir endereço para correspondência** só aparecem nos designs que os utilizam.
4. Clique em **Alterar Modelo**. Você também pode clicar duas vezes no design.
5. Escolha onde aplicar o novo design e clique em **OK**:
   - **Aplicar o modelo à publicação atual** altera a publicação que está aberta.
   - **Criar uma nova publicação com meu texto e meus elementos gráficos** abre uma segunda janela com o resultado. A publicação original não é alterada.

A galeria mostra apenas designs. Para alterar o tamanho do papel de uma publicação sem mudar o design, use [configurar página](page-setup).

## Para onde vão seu texto e suas imagens

Uma **matéria** é o texto de uma caixa de texto ou de uma sequência de caixas de texto vinculadas. Cada caixa de texto de um design tem uma **função**, que indica para que serve a caixa. As funções são:

- Título (a manchete) e subtítulo
- Cabeçalho, para letras grandes mais adiante na publicação
- Corpo do texto
- Data, que também pode conter um local ou um horário
- Endereço e dados de contato: endereço, telefone, email e endereço da Web
- Organização, que é o nome e o slogan da sua organização
- Pessoa, que é o nome e o cargo de uma pessoa
- Legenda e citação em destaque
- Endereço de correspondência, o bloco que a mala direta preenche
- Texto curto, para pequenos rótulos

Os designs que acompanham o JeffPub já têm essas funções. No caso de uma caixa que você mesmo desenhou, ou de uma vinda de um arquivo .pub, o JeffPub deduz a função pelo texto (uma data, um número de telefone), pelo estilo (Legenda ou Citação) e pelo tamanho.

Cada matéria sua vai para a caixa do novo design que tem a mesma função. Por exemplo, a matéria que você escreveu no corpo de um folheto vai para a caixa de corpo de um boletim informativo, e o nome da sua organização vai para o cabeçalho. Se o novo design tiver várias caixas com a mesma função, suas matérias as preenchem na ordem de leitura: página por página, de cima para baixo, da esquerda para a direita.

A matéria mantém as palavras, o negrito, o itálico, os links e os campos. Ela adota a fonte, a cor e o espaçamento do novo design, para combinar com o restante dele. O título de um design que usa Texto Artístico recebe o primeiro parágrafo da sua matéria de título. Os parágrafos seguintes vão para Conteúdo extra.

Suas imagens vão para as **imagens de espaço reservado** do design, que são as imagens provisórias exibidas até você adicionar as suas. A primeira imagem ocupa o primeiro espaço reservado na ordem de leitura, a próxima imagem ocupa o seguinte, e assim por diante. Cada imagem é cortada para preencher a moldura e mantém o próprio brilho, contraste e recoloração. Os espaços reservados para os quais você não tem imagem mantêm a imagem provisória do design.

Algumas coisas continuam como estão:

- O texto sobre o qual você nunca digitou é o texto de exemplo do design antigo. Ele não é transferido, e o novo design mostra o próprio texto de exemplo no lugar.
- O tamanho da página, as margens, as páginas e as páginas mestras vêm do novo design. Se a publicação tiver mais páginas que o design, ficam as páginas do design, e o restante do conteúdo vai para Conteúdo extra. O JeffPub não adiciona páginas para ele.
- Tabelas, formas, linhas, Texto Artístico e grupos de objetos criados pelo design antigo, e que você não alterou, ficam com ele. Tudo o que você desenhou e tudo o que você alterou (uma tabela com uma célula em que você digitou, uma forma cujo texto você editou, uma linha que você moveu) vai para Conteúdo extra, inteiro.
- Suas informações comerciais, propriedades do documento, estilos, lista de mala direta e objetos da área de rascunho permanecem na publicação.

## Conteúdo extra

Nada do que você criou é descartado. O conteúdo sem lugar no novo design fica no painel **Conteúdo extra**, que se abre ao lado da página quando a alteração termina. Ele lista cada matéria, imagem, tabela, forma, linha e grupo que sobrou.

- Clique em um item e depois em **Posicionar na Página** para colocá-lo na página que você está vendo. Uma matéria vira uma caixa de texto ajustada ao texto, uma tabela ou grupo volta inteiro, e uma imagem ou objeto grande demais é reduzido para caber na página. Você também pode clicar duas vezes em um item ou arrastá-lo para a página.
- Clique em um item e depois em **Descartar** para removê-lo definitivamente. (Ainda é possível desfazer.)

O painel acompanha a publicação até você fechá-la e é salvo no arquivo da publicação. (Um arquivo .pub não tem lugar para ele, por isso salvar como .pub o deixa de fora.) Para abri-lo novamente, clique em **Design da Página > Modelo > Conteúdo extra**. O botão fica esmaecido quando não sobra nada.

## Desfazer

**Desfazer** (Ctrl+Z) traz de volta o design antigo e retira novamente as sobras do Conteúdo extra. Posicionar ou descartar um item é uma etapa separada.

## Relacionados

- [Iniciar uma nova publicação](create)
- [Esquemas de cores e de fontes](color-schemes)
- [Tamanho da página, margens e layout](page-setup)
- [A guia Design da Página](tab-page-design)
