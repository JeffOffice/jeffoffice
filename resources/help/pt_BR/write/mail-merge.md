# Mala direta, envelopes e etiquetas
<!-- keywords: mala direta, mesclagem, mesclar, correspondências, lista de destinatários, carta padrão, etiquetas, envelopes, campo de mesclagem, bloco de endereço, linha de saudação, regras, se então senão, ignorar registro, próximo registro, visualizar resultados, concluir e mesclar, diretório, mensagens de email, csv, xlsx, ods, vcard -->

Uma **mala direta** faz muitas cópias de uma mesma carta, cada uma com os dados de uma pessoa diferente. Você escreve a carta uma vez. Guarda os dados das pessoas em uma **lista**. O JeffWrite coloca o nome e o endereço da primeira pessoa na primeira cópia, os da segunda pessoa na segunda cópia, e assim por diante.

As ferramentas estão na guia **Correspondências**. Para um passo a passo guiado, clique em **Correspondências > Iniciar Mala Direta > Assistente de Mala Direta Passo a Passo**.

## Escolha o que você está criando

Clique em **Correspondências > Iniciar Mala Direta** e escolha uma opção:

- **Cartas**: uma cópia da página para cada pessoa. Cada cópia começa em uma nova página.
- **Mensagens de Email**: uma mensagem para cada pessoa. Veja "Enviar mensagens de email" abaixo.
- **Envelopes** e **Etiquetas**: uma página feita para eles. Veja as seções abaixo.
- **Diretório**: uma lista de todas as pessoas, sem quebra de página entre elas. Use para uma lista telefônica ou um catálogo.
- **Limpar Tipo de Mesclagem** transforma o documento novamente em um documento comum.

O JeffWrite guarda o tipo de mesclagem e o nome da lista dentro do arquivo quando você salva. Quando você abre o arquivo novamente, a mesclagem continua lá.

## Escolha os destinatários

Um **destinatário** é uma pessoa que recebe uma cópia. Clique em **Correspondências > Selecionar Destinatários**:

- **Digitar uma Nova Lista**: digite as pessoas na janela que se abre. Clique em **Nova Entrada** para cada uma. Em seguida, o JeffWrite oferece salvar a lista como um arquivo .csv.
- **Usar uma Lista Existente**: escolha um arquivo que já seja uma lista. Pode ser um arquivo .csv ou um arquivo de texto com tabulações (uma linha para cada pessoa, e a primeira linha contém os nomes das colunas), uma planilha (.xlsx, .xls ou .ods, do JeffCrunch ou de outro programa), ou um arquivo de contatos (.vcf).
- **Selecionar dos Contatos**: escolha um arquivo de contatos (.vcf). A maioria das agendas e celulares pode salvar seus contatos em um arquivo desse tipo.

Depois, clique em **Correspondências > Editar Lista de Destinatários** para conferir a lista:

- Desmarque uma linha para deixar essa pessoa de fora.
- Clique em um cabeçalho de coluna para classificar.
- Digite em **Filtrar destinatários** para mostrar apenas as linhas que têm as suas palavras.
- **Diferenciar destinatários por** escolhe a coluna que distingue uma pessoa de outra, como um endereço de email. Quando você abre o documento novamente e a lista é lida, as pessoas que você desmarcou continuam de fora, mesmo que a lista tenha outra ordem ou novas pessoas. Com **Colocar na lista**, elas são lembradas pelo número da linha no arquivo da lista, mesmo que você tenha classificado ou filtrado as linhas nesta janela. Se a lista mudou de forma que isso não se aplique mais, o JeffWrite avisa e inclui todas as pessoas que não conseguiu relacionar. O documento guarda uma forma codificada de cada endereço, não o endereço em si, mas uma pessoa que tenha o documento e adivinhe um endereço ainda pode testar esse palpite. **Inspecionar Documento** (Arquivo > Informações) remove o registro de quem foi deixado de fora.
- **Localizar Duplicatas** desmarca as repetições.
- **Validar Endereços** colore as linhas que não têm endereço ou que têm um CEP com aparência incorreta.

## Coloque os dados na carta

Um **campo de mesclagem** é um espaço reservado. Ele mostra o nome de uma coluna entre sinais de aspas angulares duplos, como «Nome». Em cada cópia, ele é substituído pelo dado daquela pessoa.

Clique onde o dado deve aparecer e use **Correspondências > Gravar e Inserir Campos**:

- **Inserir Campo de Mesclagem** lista as colunas da sua lista. Escolha uma.
  - **Código de Barras a Partir de Campo**, na mesma lista, insere um código de barras cujo valor é uma coluna, como um número de peça ou o ISBN de 13 dígitos de um livro. Escolha a coluna, depois o tipo, e veja na janela **Inserir Código de Barras** (o que você digitar ali é apenas um exemplo). A janela oferece somente o que um campo pode conter: o tipo, o tamanho, a altura das barras, os dígitos e o nível do QR. Ela deixa de fora o preço de um livro, um complemento, um caractere de verificação e o fundo claro, que um campo não consegue indicar; o ISBN de um livro é um EAN-13. No documento, o campo aparece como um código de barras, com um exemplo do tipo até a mesclagem; para mudar o tipo ou a aparência, clique duas vezes nele ou clique com o botão direito e escolha **Editar Código de Barras**. Ao mesclar, a cópia de cada pessoa recebe um código de barras com o valor próprio dela, desenhado como imagem na linha, para que seja impresso e salvo em PDF como qualquer imagem. O valor é apenas um dado: aspas ou barras invertidas nele não alteram nada. Se um valor estiver vazio, não há código de barras. Se um código não puder conter o valor (uma letra em um EAN-13, ou mais de 500 caracteres em Code 128), a cópia mostra uma breve mensagem entre colchetes no lugar dele, e as demais cópias não são afetadas. Uma mesclagem gera no máximo 2.000 códigos de barras, contando cada valor que não pode ser representado; mescle menos pessoas se o programa disser que há muitos. Em um `.docx` salvo, o campo é um campo DISPLAYBARCODE com a coluna dentro, por isso ele é mantido ao salvar e abrir o arquivo novamente. Um campo com um valor digitado, ou de um tipo que o JeffWrite não desenha (UPC-E, por exemplo), permanece como está. Mensagens de email recebem o valor como texto.
- **Bloco de Endereço** insere um endereço postal completo: o nome, a empresa, a rua e a cidade. Escolha as linhas que quiser. Uma linha sem conteúdo é omitida. Você também pode indicar um país a ser omitido, como o seu próprio. A **Visualização** mostra o endereço da primeira pessoa.
- **Linha de Saudação** insere uma linha como "Prezado Dr. Fenwick,". Você escolhe as palavras. A primeira opção, "Prezado Dr. Fenwick,", usa o título da pessoa. Se a pessoa não tiver título, usa o primeiro e o último nome, como em "Dear Nora Pell,". Se a pessoa não tiver nome, a saudação diz outra coisa, como "Prezado Senhor ou Senhora,", que você pode alterar.
- **Corresponder Campos** informa ao bloco de endereço e à linha de saudação qual coluna contém cada parte, quando os nomes das suas colunas são incomuns.

## Regras

Uma **regra** faz uma cópia ser diferente para algumas pessoas. Clique em **Regras** e escolha uma:

- **Se…Então…Senão**: mostra um texto quando uma coluna tem um valor, e outro texto quando não tem. Por exemplo, se o Estado for "SP", escreva "Bem-vindo de volta".
- **Ignorar Registro Se**: deixa uma pessoa de fora quando uma coluna atende a um teste definido por você, como quando o sobrenome está vazio.
- **Próximo Registro** e **Próximo Registro Se**: usam os dados da pessoa seguinte no mesmo lugar. As etiquetas usam isso para colocar uma pessoa diferente em cada etiqueta.
- **Número do Registro** insere o número da linha da pessoa na lista. **Número da Sequência de Mesclagem** conta as cópias feitas até o momento.
- **Perguntar** e **Preenchimento**: fazem uma pergunta quando você termina a mesclagem. Perguntar guarda a resposta com um nome. Preenchimento insere a resposta no texto.
- **Definir Indicador**: atribui um valor a um nome. Uma referência cruzada ao nome mostra o valor.

## Visualize os resultados

Clique em **Correspondências > Visualizar Resultados**. Abre-se um painel que mostra a página de uma pessoa. Use **Primeiro Registro**, **Registro Anterior**, **Próximo Registro** e **Último Registro** para percorrer as pessoas, ou digite um número. **Localizar Destinatário** vai até a próxima pessoa que tenha as suas palavras em algum dado.

**Realçar Campos de Mesclagem** sombreia em cinza os campos de mesclagem da página, para que você veja onde estão. Outros campos, como números de página, não são sombreados. O sombreamento aparece na tela e nunca é impresso.

Seu documento original não é alterado. **Verificar Erros** avisa quando a lista não tem uma coluna para um campo, ou quando nenhuma pessoa está incluída.

## Conclua a mesclagem

Clique em **Correspondências > Concluir e Mesclar**:

- **Editar Documentos Individuais** cria um novo documento com a cópia de cada pessoa. Você pode alterá-lo antes de imprimir.
- **Imprimir Documentos** cria esse documento e abre a página **Imprimir**.
- **Salvar como PDF** salva todas as cópias em um único arquivo PDF.
- **Enviar Mensagens de Email** cria um arquivo de mensagem .eml para cada pessoa, em uma pasta que você escolhe. Escolha a coluna que contém os endereços de email e digite um assunto. O JeffWrite não envia as mensagens. Abra os arquivos no seu programa de email.
- **Salvar Cada Um como Arquivo** cria um arquivo para cada pessoa, em uma pasta que você escolhe, como um arquivo .docx ou PDF. Nomeie os arquivos com colunas entre chaves, como {Sobrenome}, {Nome}; um nome usado duas vezes recebe um número. Uma caixa mostra os primeiros nomes e quantos arquivos serão criados. Você é consultado antes de mais de 200 arquivos, e uma vez se já existirem arquivos com os mesmos nomes na pasta. Clique em **Cancelar** para parar entre um arquivo e outro: os arquivos já criados são mantidos; fechar a janela tem o mesmo efeito. Um arquivo que é um link para outro arquivo é substituído por um novo arquivo, e o arquivo para o qual ele apontava não é alterado. Os arquivos contêm a carta de uma só pessoa: não contêm a lista, o local dela nem quem foi deixado de fora. Uma mesclagem gera no máximo 100.000 arquivos.

As pessoas que você desmarcou na lista ficam de fora.

Um arquivo de mensagem só é criado para uma pessoa cujo endereço seja um único endereço de email. Se um endereço tiver dois (com vírgula ou ponto e vírgula entre eles), tiver um nome junto, ou não tiver domínio, essa pessoa não recebe mensagem, e o JeffWrite informa quantas foram. Um assunto longo é salvo em partes que os programas de email juntam novamente.

Uma mesclagem mostra uma janela de progresso, e **Cancelar** a interrompe. Se uma regra fizer uma pergunta (Perguntar ou Preenchimento), você é consultado uma vez para cada pergunta diferente, até 20 perguntas. Depois disso, as demais usam suas respostas padrão. Se você cancelar uma pergunta, a mesclagem inteira para. Uma mesclagem para e informa isso se for gerar mais de 10.000 páginas, para que um documento muito longo com uma lista muito grande não leve horas. Mescle uma parte da lista de cada vez.

## Envelopes

Clique em **Correspondências > Envelopes**. Digite o endereço de entrega e o seu endereço de remetente, e escolha o tamanho do envelope na lista. Os tamanhos são listados pela largura e pela altura. Clique em **Adicionar ao Documento** para colocar o envelope em uma página própria no início do documento, ou em **Imprimir** para imprimi-lo. A página tem a largura do lado maior do envelope, que é como a maioria das impressoras recebe um envelope.

Para criar envelopes para uma mesclagem, clique em **Iniciar Mala Direta > Envelopes**. Cada envelope terá um bloco de endereço.

## Etiquetas

Clique em **Correspondências > Etiquetas**. Digite o endereço e escolha a folha na lista. Uma folha é identificada pela quantidade de etiquetas que tem e pelo tamanho de cada uma. Se a sua folha não estiver na lista, escolha **Sua própria folha** e digite as medidas: quantas etiquetas cabem na horizontal e na vertical, o tamanho de uma etiqueta, o espaço acima e ao lado da primeira etiqueta, e a distância entre as etiquetas (em centímetros).

- **Página completa com a mesma etiqueta** coloca o mesmo endereço em todas as etiquetas da folha.
- **Etiqueta única** coloca o endereço em uma só etiqueta, para aproveitar uma folha que já foi parcialmente usada. Indique a linha e a coluna.

Clique em **Novo Documento** para criar um documento que você pode ver e alterar, ou em **Imprimir** para imprimi-lo.

Para criar etiquetas para uma mesclagem, clique em **Iniciar Mala Direta > Etiquetas**. A primeira etiqueta terá um bloco de endereço, e cada uma das demais começará com uma regra de **Próximo Registro**. Se você alterar a primeira etiqueta, clique em **Atualizar Etiquetas** para copiar a alteração para todas as outras.

## O que o arquivo guarda

O tipo de mesclagem, o nome e o local da lista, o assunto e a coluna de email são salvos no seu arquivo .docx. As pessoas que você desmarcou também são salvas, como códigos curtos da coluna que você escolheu, não como nomes ou endereços (somente um arquivo .docx guarda isso; Verificar Compatibilidade informa isso para os demais formatos). As suas escolhas de Corresponder Campos também são salvas. Uma lista é lida novamente do seu arquivo cada vez. Se o arquivo não estiver mais no lugar, o JeffWrite procura ao lado do seu documento e, depois, pede que você o escolha.

Uma lista pode ter até 1.000 colunas e 4.000.000 de células no total, e uma mesclagem usa as primeiras 100.000 pessoas. Se um arquivo tiver mais, o JeffWrite usa o que cabe e avisa você. Linhas sem nenhum conteúdo são deixadas de fora da lista.

## Veja também

- [Usar campos](fields)
- [Imprimir](print)
- [Criar e alterar tabelas](tables)
