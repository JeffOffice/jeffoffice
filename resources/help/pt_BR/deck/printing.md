# Impressão, PDF e imagens de slides
<!-- keywords: imprimir, impressão, impressora, imprimir, cópias, agrupar, agrupadas, seleção, última página primeiro, inverter, zoom, slides, páginas de anotações, folhetos, estrutura de tópicos, pdf, exportar, imagem, png, jpeg, jpg, figura, intervalo, slides ocultos, moldura, escala de cinza, preto e branco, visualizar impressão, papel, carta, a4, senha, criptografar, proteger com senha, tópicos -->

Você pode imprimir sua apresentação, salvá-la como PDF ou salvar um slide como imagem. As três opções estão na tela Arquivo.

## Imprimir

1. Pressione Ctrl+P ou clique em **Arquivo > Imprimir**.
2. Ao lado do botão **Imprimir**, defina **Cópias**. Escolha uma **Impressora**: a lista termina com **Salvar como PDF**, que cria um arquivo PDF em vez de papel. Com mais de uma cópia, **Agrupadas** imprime o conjunto completo novamente para cada cópia (1, 2, 3, depois 1, 2, 3), e com a opção desativada as cópias de cada página saem juntas (1, 1, 2, 2, 3, 3). Um arquivo PDF é criado uma única vez, qualquer que seja o número de cópias. **Propriedades da Impressora…** abre a janela de impressão do sistema para essa impressora, com suas próprias opções (a bandeja, a qualidade); o que você escolher ali é mantido para o trabalho, e as cópias, o agrupamento, o papel e a escala de cinza voltam para esta página. **Papel** define o tamanho da folha; **Como indicam os slides** é a escolha usual.
3. Em **Imprimir o quê**, escolha o que será impresso: **Slides**, **Páginas de Anotações**, **Estrutura de Tópicos** ou **Folhetos** com 1, 2, 3, 4, 6 ou 9 slides por página.
4. Para folhetos com 4 ou mais slides, **Ordem** indica se os slides seguem da esquerda para a direita e depois para baixo (**Horizontal**) ou de cima para baixo e depois para o lado (**Vertical**).
5. Em **Intervalo**, escolha **Todos os slides**, **Slide atual** ou **Seleção** (os slides selecionados no classificador de slides ou o slide em que você está, quando não estiver no classificador), ou escolha **Slides** e digite os números na caixa: números de slides e intervalos separados por vírgulas, como 1-3, 5.
6. Marque o que precisar: **Última página primeiro** (as folhas saem em ordem inversa, para que a pilha fique na ordem certa; na impressão frente e verso, a frente fica junto com o verso, então os slides 1 a 4 saem como 3 e 4, depois 1 e 2; a opção fica esmaecida quando você imprime frente e verso manualmente), **Imprimir slides ocultos**, **Emoldurar slides** (uma linha fina ao redor de cada slide) e **Escala de cinza**.
7. Em **Lados**, escolha **Imprimir em um lado**, **Imprimir frente e verso, virar pela borda longa** (páginas que viram como um livro), **Imprimir frente e verso, virar pela borda curta** (páginas que viram como um bloco de anotações) ou **Imprimir frente e verso manualmente**. As duas primeiras opções ficam esmaecidas quando sua impressora não consegue imprimir frente e verso. Aponte para uma delas para ver o motivo. **Manualmente** é para esse tipo de impressora: o JeffDeck imprime a primeira, a terceira, a quinta e assim por diante, depois uma mensagem pede que você vire a pilha e a coloque de volta na impressora. Ao clicar em **OK**, ele imprime as outras, da última página para a primeira. Com uma única página, não há nada para virar. O JeffDeck lembra sua escolha.
8. Clique em **Imprimir**. Para um arquivo PDF, o JeffDeck pede um nome. Um arquivo PDF não tem lados.

A visualização à direita mostra cada página que será impressa, com setas para ir de uma página à outra. **Zoom** acima dela aumenta a página além do tamanho que cabe na tela; role para ver o restante. Ela é desenhada da mesma forma como a página será impressa. O JeffDeck lembra suas escolhas para a próxima vez.

## O que cada opção imprime

- **Slides** imprime um slide em cada página, na forma do próprio slide. O papel é virado para o lado quando o slide é largo.
- **Páginas de Anotações** coloca um slide no alto da página, com as anotações do orador abaixo dele.
- **Folhetos** colocam uma grade de slides em cada página, com cabeçalho e rodapé contendo o título, a data e o número da página. Com 3 por página, há linhas ao lado de cada slide para anotações.
- **Estrutura de Tópicos** imprime o número e o título de cada slide, seguidos do texto do corpo, recuado conforme o nível.

Páginas de anotações, folhetos e a estrutura de tópicos são impressos em papel carta ou A4 em pé, conforme o padrão da sua região. Os slides ocultos ficam de fora, a menos que você marque **Imprimir slides ocultos**.

## Salvar como PDF

Clique em **Arquivo > Exportar > Exportar para PDF**. Uma janela pergunta o que incluir: as mesmas opções de **Imprimir o quê**, **Ordem** e **Intervalo** da impressão, e as mesmas três caixas de seleção. Ela mostra quantas páginas o arquivo terá. Clique em **Exportar…** e depois escolha um nome e um local.

O PDF tem texto real, com as fontes embutidas, então fica igual em qualquer computador. Seu título é o título da apresentação ou o nome do arquivo.

**Tags de estrutura do documento para acessibilidade** (ativada por padrão) cria um PDF *marcado*: o título de cada slide vira um título, os objetos ficam na ordem em que um leitor de tela os lê (a ordem de leitura do slide), imagens e formas têm texto alternativo, e uma tabela é uma tabela. **Criar indicadores** (ativada por padrão) coloca um indicador no PDF para cada slide, com o título do slide, ou "Slide 3" para um slide sem título. O layout de estrutura de tópicos não tem nem um nem outro.

Marque **PDF/A, para guardar por muito tempo** para criar um arquivo PDF/A-1b. Todas as fontes ficam dentro do arquivo, e os efeitos de transparência são nivelados, para que os slides continuem iguais por anos. As tags e os indicadores permanecem no arquivo. Um PDF com senha não pode ser um arquivo PDF/A, então escolher uma senha desativa o PDF/A.

## Enviar uma apresentação por email

Clique em **Arquivo > Exportar > Criar um Email**. Escolha a apresentação como arquivo ou como PDF. O JeffDeck cria um arquivo de mensagem (.eml) com ela anexada e pergunta onde salvá-lo. Nada é enviado. Abra o arquivo no seu programa de email, adicione os destinatários e envie. Uma apresentação que tem senha é anexada com a senha ainda aplicada. Um PDF pode ter senha própria.

## Proteger um PDF com senha

Na janela **Exportar para PDF**, clique em **Criptografar com senha…**. Digite a senha duas vezes. Quem abrir o PDF terá de digitá-la. A senha realmente protege o arquivo: as páginas são embaralhadas com criptografia forte (AES-256), e ninguém consegue lê-las sem ela.

Você também pode marcar **Restringir impressão, cópia e edição**, informar uma segunda senha e escolher o que é permitido. Essas restrições são apenas um pedido. A maioria dos leitores de PDF as respeita, mas elas não impedem quem quer contorná-las, então não conte com elas para manter um arquivo em sigilo.

Uma senha esquecida não pode ser recuperada, então guarde-a em um lugar seguro. A senha é usada somente para esse PDF e nunca é salva.

Ao inserir como imagem um PDF protegido por senha, o JeffDeck pede a senha. Se estiver errada, ele pede novamente; clique em **Cancelar** para parar. O slide recebe uma cópia da página que não precisa de senha. Se o proprietário do PDF pedir que ele não seja copiado, o JeffDeck o insere mesmo assim e mostra um aviso informando isso; esse pedido não é um bloqueio.

## Salvar um slide como imagem

Clique em **Arquivo > Exportar > Exportar Slide como Imagem**. O JeffDeck salva o slide em que você está como uma imagem PNG ou JPEG, com 1920 pixels de largura, em fundo branco. Ele sugere o nome da apresentação e o número do slide como nome do arquivo.

## Veja também

- [A tela Arquivo](file-screen)
- [Anotações do orador](notes)
- [Adicionar, alterar e excluir slides](slides)
