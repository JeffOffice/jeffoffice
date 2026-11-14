# Mala direta, envelopes e etiquetas
<!-- keywords: mala direta, mesclagem, correspondências, destinatários, lista de endereços, carta modelo, etiquetas, envelopes, campo de mesclagem, bloco de endereço, linha de saudação, regras, se então senão, ignorar registro, próximo registro, visualizar resultados, concluir e mesclar, diretório, mensagens de email, csv, xlsx, ods, vcard -->

Uma **mala direta** cria muitas cópias de uma mesma carta, cada uma com os dados de uma pessoa diferente. Você escreve a carta uma única vez. Você guarda os dados das pessoas em uma **lista**. O JeffWrite coloca o nome e o endereço da primeira pessoa na primeira cópia, os da segunda pessoa na segunda cópia, e assim por diante.

As ferramentas estão na guia **Correspondências**. Para um caminho guiado, clique em **Correspondências > Iniciar Mala Direta > Assistente de Mala Direta Passo a Passo**.

## Escolha o que você está criando

Clique em **Correspondências > Iniciar Mala Direta** e escolha uma opção:

- **Cartas**: uma cópia da página para cada pessoa. Cada cópia começa em uma nova página.
- **Mensagens de Email**: uma mensagem para cada pessoa. Veja "Enviar mensagens de email" mais abaixo.
- **Envelopes** e **Etiquetas**: uma página feita para eles. Veja as seções abaixo.
- **Diretório**: uma única lista de todas as pessoas, sem quebra de página entre elas. Use para uma lista telefônica ou um catálogo.
- **Limpar Tipo de Mesclagem** transforma o documento em um documento comum novamente.

O JeffWrite guarda o tipo de mesclagem e o nome da lista dentro do arquivo quando você o salva. Ao abrir o arquivo novamente, a mesclagem continua lá.

## Escolha os destinatários

Um **destinatário** é uma pessoa que recebe uma cópia. Clique em **Correspondências > Selecionar Destinatários**:

- **Digitar uma Nova Lista**: digite as pessoas na janela que abrir. Clique em **Nova Entrada** para cada uma delas. Em seguida, o JeffWrite oferece salvar a lista como um arquivo .csv.
- **Usar uma Lista Existente**: escolha um arquivo que já seja uma lista. Pode ser um arquivo .csv ou um arquivo de texto com tabulações (uma linha para cada pessoa, e a primeira linha contém os nomes das colunas), uma planilha (.xlsx, .xls ou .ods, do JeffCrunch ou de outro programa) ou um arquivo de contatos (.vcf).
- **Selecionar dos Contatos**: escolha um arquivo de contatos (.vcf). A maioria das agendas e dos celulares pode salvar os contatos nesse formato.

Depois, clique em **Correspondências > Editar Lista de Destinatários** para conferir a lista:

- Desmarque uma linha para deixar essa pessoa de fora.
- Clique em um cabeçalho de coluna para classificar.
- Digite em **Filtrar destinatários** para mostrar apenas as linhas que contêm as suas palavras.
- **Localizar Duplicatas** desmarca as repetições.
- **Validar Endereços** destaca as linhas que não têm endereço ou que têm um CEP com aparência incorreta.

## Coloque os dados na carta

Um **campo de mesclagem** é um espaço reservado. Ele mostra o nome de uma coluna entre sinais de aspas angulares duplos, como «Nome». Em cada cópia, ele é substituído pelo dado da pessoa.

Clique onde o dado deve aparecer e use **Correspondências > Gravar e Inserir Campos**:

- **Inserir Campo de Mesclagem** lista as colunas da sua lista. Escolha uma.
  - **Código de Barras a Partir de Campo**, na mesma lista, insere um código de barras cujo valor é uma coluna, como um número de peça ou o ISBN de 13 dígitos de um livro. Escolha a coluna, depois o tipo, e observe a janela **Inserir Código de Barras** (o que você digitar ali é apenas um exemplo). A janela oferece apenas o que um campo pode conter: o tipo, o tamanho, a altura das barras, os dígitos e o nível do QR. Ela não inclui o preço de um livro, um complemento, um caractere de verificação nem o fundo transparente, que um campo não consegue indicar; o ISBN de um livro é um EAN-13. No documento, o campo aparece como um código de barras, com um exemplo do tipo até você fazer a mesclagem; para mudar o tipo ou a aparência, clique duas vezes sobre ele ou clique com o botão direito e escolha **Editar Código de Barras**. Ao mesclar, a cópia de cada pessoa recebe um código de barras com o valor próprio dela, desenhado como uma imagem na linha, para que seja impresso e salvo em PDF como qualquer imagem. O valor é apenas um dado: aspas ou barras invertidas nele não alteram nada. Se um valor estiver vazio, não há código de barras. Se um código não puder conter o valor (uma letra em um EAN-13, ou mais de 500 caracteres em Code 128), a cópia mostra uma breve mensagem entre colchetes no lugar dele, e as outras cópias não são afetadas. Uma mesclagem gera no máximo 2.000 códigos de barras, contando cada valor que não pode ser representado; se a mensagem indicar que há muitos, mescle menos pessoas. Em um `.docx` salvo, o campo é um campo DISPLAYBARCODE com a coluna dentro dele, de modo que ele é mantido ao salvar e abrir o arquivo novamente. Um campo com um valor digitado diretamente, ou de um tipo que o JeffWrite não desenha (UPC-E, por exemplo), é mantido como está. Nas mesclagens de email, o valor é inserido como texto.
- **Bloco de Endereço** insere um endereço postal completo: o nome, a empresa, a rua e a cidade. Escolha as linhas que deseja. Uma linha sem conteúdo é omitida. Você também pode indicar um país para omitir, como o seu próprio. A **Visualização** mostra o endereço da primeira pessoa.
- **Linha de Saudação** insere uma linha como "Prezado Dr. Fenwick,". Você escolhe as palavras. A primeira opção, "Prezado Dr. Fenwick,", usa o título da pessoa. Se a pessoa não tiver título, a saudação usa o nome e o sobrenome, como em "Prezado Nora Pell,". Se a pessoa não tiver nome, a saudação diz outra coisa, como "Prezado Senhor ou Senhora,", que você pode alterar.
- **Correlacionar Campos** informa ao bloco de endereço e à linha de saudação qual coluna contém cada parte, quando os nomes das suas colunas são incomuns.

## Regras

Uma **regra** faz uma cópia ser diferente para algumas pessoas. Clique em **Regras** e escolha uma opção:

- **Se…Então…Senão**: mostra um texto quando uma coluna tem um valor e outro texto quando não tem. Por exemplo, se Estado for "SP", escreva "Bem-vindo".
- **Ignorar Registro Se**: deixa uma pessoa de fora quando uma coluna atende a um teste definido por você, como quando o sobrenome está vazio.
- **Próximo Registro** e **Próximo Registro Se**: usam os dados da pessoa seguinte no mesmo lugar. As etiquetas usam isso para colocar uma pessoa diferente em cada etiqueta.
- **Número do Registro de Mesclagem** insere o número da linha da pessoa na lista. **Número de Sequência de Mesclagem** conta as cópias feitas até o momento.
- **Perguntar** e **Preenchimento**: pedem uma resposta a você ao final da mesclagem. Perguntar guarda a resposta com um nome. Preenchimento insere a resposta no texto.
- **Definir Indicador**: dá a um nome um valor. Uma referência cruzada a esse nome mostra o valor.

## Visualize os resultados

Clique em **Correspondências > Visualizar Resultados**. Abre-se um painel que mostra a página de uma pessoa. Use **Primeiro Registro**, **Registro Anterior**, **Próximo Registro** e **Último Registro** para percorrer as pessoas, ou digite um número. **Localizar Destinatário** vai até a próxima pessoa que tem as suas palavras em algum dado.

**Realçar Campos de Mesclagem** sombreia de cinza os campos de mesclagem na página, para que você veja onde estão. Outros campos, como números de página, não são sombreados. O sombreamento aparece na tela e nunca é impresso.

O seu documento não é alterado. **Verificar Erros** avisa quando a lista não tem uma coluna para um campo, ou quando nenhuma pessoa está incluída.

## Concluir a mesclagem

Clique em **Correspondências > Concluir e Mesclar**:

- **Editar Documentos Individuais** cria um novo documento com a cópia de cada pessoa. Você pode alterá-lo antes de imprimir.
- **Imprimir Documentos** cria esse documento e abre a página **Imprimir**.
- **Salvar como PDF** salva todas as cópias em um único arquivo PDF.
- **Enviar Mensagens de Email** cria um arquivo de mensagem .eml para cada pessoa, em uma pasta que você escolhe. Escolha a coluna que contém os endereços de email e digite um assunto. O JeffWrite não envia as mensagens. Abra os arquivos no seu programa de email.

As pessoas que você desmarcou na lista ficam de fora.

Um arquivo de mensagem só é criado para uma pessoa cujo endereço contém um único endereço de email. Se um endereço contiver dois (com vírgula ou ponto e vírgula entre eles), tiver um nome junto, ou não tiver domínio, essa pessoa não recebe mensagem, e o JeffWrite informa quantas foram. Um assunto longo é salvo em partes, que os programas de email juntam novamente.

A mesclagem mostra uma janela de andamento, e **Cancelar** a interrompe. Se uma regra fizer uma pergunta (Perguntar ou Preenchimento), você responde a cada pergunta diferente uma única vez, até 20 perguntas. Depois disso, as demais usam as respostas padrão. Se você cancelar uma pergunta, a mesclagem inteira para. A mesclagem para e informa isso se for gerar mais de 10.000 páginas, para que um documento muito longo com uma lista muito grande não leve horas. Faça a mesclagem de parte da lista por vez.

## Envelopes

Clique em **Correspondências > Envelopes**. Digite o endereço de entrega e o seu endereço de remetente, e escolha o tamanho do envelope na lista. Os tamanhos são indicados pela largura e pela altura. Clique em **Adicionar ao Documento** para colocar o envelope em uma página própria no início do documento, ou em **Imprimir** para imprimi-lo. A página tem a largura do lado maior do envelope, que é como a maioria das impressoras recebe um envelope.

Para criar envelopes para uma mesclagem, clique em **Iniciar Mala Direta > Envelopes**. Cada envelope tem um bloco de endereço.

## Etiquetas

Clique em **Correspondências > Etiquetas**. Digite o endereço e escolha a folha na lista. Uma folha é identificada pela quantidade de etiquetas que contém e pelo tamanho de cada uma. Se a sua folha não estiver na lista, escolha **Sua própria folha** e digite as medidas: quantas etiquetas cabem na largura e na altura, o tamanho de uma etiqueta, o espaço acima e ao lado da primeira etiqueta, e a distância entre as etiquetas. Em centímetros, por exemplo, uma etiqueta comum de 2,5 cm por 5 cm.

- **Página inteira com a mesma etiqueta** coloca o mesmo endereço em todas as etiquetas da folha.
- **Etiqueta única** coloca o endereço em uma só etiqueta, para aproveitar uma folha que já foi parcialmente usada. Informe a linha e a coluna.

Clique em **Novo Documento** para criar um documento que você possa ver e alterar, ou em **Imprimir** para imprimi-lo.

Para criar etiquetas para uma mesclagem, clique em **Iniciar Mala Direta > Etiquetas**. A primeira etiqueta tem um bloco de endereço, e cada uma das outras começa com uma regra de **Próximo Registro**. Se você alterar a primeira etiqueta, clique em **Atualizar Etiquetas** para copiar a alteração para todas as outras.

## O que o arquivo guarda

O tipo de mesclagem, o nome e o local da lista, o assunto e a coluna de email são salvos no seu arquivo .docx. Quais pessoas você desmarcou e as suas escolhas em Correlacionar Campos não são salvas. Elas são pedidas novamente depois que você abre o arquivo. A lista é lida de novo a partir do seu arquivo a cada vez. Se o arquivo não estiver mais onde estava, o JeffWrite procura ao lado do seu documento e, em seguida, pede que você o escolha.

Uma lista pode ter até 1.000 colunas e 4.000.000 de células ao todo, e uma mesclagem usa as primeiras 100.000 pessoas. Se um arquivo tiver mais do que isso, o JeffWrite usa o que cabe e informa. Linhas sem conteúdo são omitidas da lista.

## Veja também

- [Usar campos](fields)
- [Imprimir](print)
- [Criar e alterar tabelas](tables)
