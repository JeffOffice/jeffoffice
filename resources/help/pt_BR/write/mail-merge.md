# Mala direta, envelopes e etiquetas
<!-- keywords: mala direta, mesclagem, correspondências, destinatários, lista de endereços, carta modelo, etiquetas, envelopes, campo de mesclagem, bloco de endereço, linha de saudação, regras, se então senão, ignorar registro, próximo registro, visualizar resultados, concluir e mesclar, catálogo, mensagens de email, csv, xlsx, ods, vcard -->

Uma **mala direta** cria muitas cópias de uma mesma carta, cada uma com os dados de uma pessoa diferente. Você escreve a carta uma única vez e guarda os dados das pessoas em uma **lista**. O JeffWrite coloca o nome e o endereço da primeira pessoa na primeira cópia, os da segunda pessoa na segunda cópia, e assim por diante.

As ferramentas estão na guia **Correspondências**. Para um passo a passo guiado, clique em **Correspondências > Iniciar Mala Direta > Assistente de Mala Direta Passo a Passo**.

## Escolha o que você está criando

Clique em **Correspondências > Iniciar Mala Direta** e escolha uma opção:

- **Cartas**: uma cópia da página para cada pessoa. Cada cópia começa em uma nova página.
- **Mensagens de Email**: uma mensagem para cada pessoa. Veja "Enviar mensagens de email" mais abaixo.
- **Envelopes** e **Etiquetas**: uma página preparada para eles. Veja as seções abaixo.
- **Catálogo**: uma lista de todas as pessoas, sem quebra de página entre elas. Use para uma lista telefônica ou um catálogo.
- **Limpar Tipo de Mesclagem** transforma o documento em um documento comum novamente.

O JeffWrite guarda o tipo de mesclagem e o nome da lista dentro do arquivo quando você salva. Ao abrir o arquivo outra vez, a mesclagem continua lá.

## Escolha os destinatários

Um **destinatário** é uma pessoa que recebe uma cópia. Clique em **Correspondências > Selecionar Destinatários**:

- **Digitar uma Nova Lista**: digite as pessoas na janela que se abre. Clique em **Nova Entrada** para cada uma. Em seguida, o JeffWrite oferece salvar a lista como um arquivo .csv.
- **Usar uma Lista Existente**: escolha um arquivo que já seja uma lista. Pode ser um arquivo .csv ou um arquivo de texto com tabulações (uma linha para cada pessoa, e a primeira linha com os nomes das colunas), uma planilha (.xlsx, .xls ou .ods, do JeffCrunch ou de outro programa) ou um arquivo de contatos (.vcf).
- **Selecionar dos Contatos**: escolha um arquivo de contatos (.vcf). A maioria das agendas e celulares pode salvar os contatos em um arquivo desse tipo.

Depois, clique em **Correspondências > Editar Lista de Destinatários** para conferir a lista:

- Desmarque uma linha para deixar essa pessoa de fora.
- Clique em um cabeçalho de coluna para classificar.
- Digite em **Filtrar destinatários** para mostrar apenas as linhas que contenham as suas palavras.
- **Localizar Duplicatas** desmarca os registros repetidos.
- **Validar Endereços** destaca em cor as linhas sem endereço ou com um CEP que parece incorreto.

## Coloque os dados na carta

Um **campo de mesclagem** é um espaço reservado. Ele mostra o nome de uma coluna entre sinais de aspas angulares duplos, como «Nome». Em cada cópia, ele é substituído pelo dado da pessoa.

Clique onde o dado deve aparecer e use **Correspondências > Gravar e Inserir Campos**:

- **Inserir Campo de Mesclagem** lista as colunas da sua lista. Escolha uma.
- **Bloco de Endereço** insere um endereço postal completo: o nome, a empresa, a rua e a cidade. Escolha as linhas que deseja. Uma linha sem conteúdo é omitida. Você também pode indicar um país para omitir, como o seu próprio. A **Visualização** mostra o endereço da primeira pessoa.
- **Linha de Saudação** insere uma linha como "Prezado Dr. Fenwick,". Você escolhe as palavras. A primeira opção, "Prezado Dr. Fenwick,", usa o título da pessoa. Se uma pessoa não tiver título, a saudação usa o nome e o sobrenome, como em "Prezada Nora Pell,". Se uma pessoa não tiver nome, a saudação diz outra coisa, como "Prezado Senhor ou Senhora,", que você pode alterar.
- **Correlacionar Campos** informa ao bloco de endereço e à linha de saudação qual coluna contém cada parte, quando os nomes das colunas são incomuns.

## Regras

Uma **regra** faz com que a cópia seja diferente para algumas pessoas. Clique em **Regras** e escolha uma opção:

- **Se…Então…Senão**: mostra um texto quando uma coluna tem conteúdo e outro texto quando não tem. Por exemplo, se Estado for "CA", escreva "Bem-vindo ao lar".
- **Ignorar Registro Se**: deixa uma pessoa de fora quando uma coluna tem determinado conteúdo, como quando o sobrenome está vazio.
- **Próximo Registro** e **Próximo Registro Se**: usam os dados da próxima pessoa no mesmo lugar. As etiquetas usam essa regra para colocar uma pessoa diferente em cada etiqueta.
- **Nº do Registro de Mesclagem** insere o número da linha da pessoa na lista. **Nº da Sequência de Mesclagem** conta as cópias feitas até o momento.
- **Perguntar** e **Preenchimento**: fazem uma pergunta a você ao concluir a mesclagem. Perguntar guarda a resposta com um nome. Preenchimento coloca a resposta no texto.
- **Definir Indicador**: dá um valor a um nome. Uma referência cruzada a esse nome mostra o valor.

## Visualize os resultados

Clique em **Correspondências > Visualizar Resultados**. Abre-se um painel que mostra a página de uma pessoa. Use **Primeiro Registro**, **Registro Anterior**, **Próximo Registro** e **Último Registro** para percorrer as pessoas, ou digite um número. **Localizar Destinatário** pula para a próxima pessoa que tenha as suas palavras em algum dado.

**Realçar Campos de Mesclagem** sombreia em cinza os campos de mesclagem da página, para que você veja onde estão. Outros campos, como números de página, não são sombreados. O sombreamento aparece na tela e nunca é impresso.

O seu documento não é alterado. **Verificar Erros** avisa quando a lista não tem uma coluna para um campo ou quando ninguém está incluído.

## Conclua a mesclagem

Clique em **Correspondências > Concluir e Mesclar**:

- **Editar Documentos Individuais** cria um novo documento com a cópia de cada pessoa. Você pode alterá-lo antes de imprimir.
- **Imprimir Documentos** cria esse documento e abre a página **Imprimir**.
- **Salvar como PDF** salva todas as cópias em um único arquivo PDF.
- **Enviar Mensagens de Email** cria um arquivo de mensagem .eml para cada pessoa, em uma pasta que você escolhe. Escolha a coluna que contém os endereços de email e digite um assunto. O JeffWrite não envia as mensagens. Abra os arquivos no seu programa de email.

As pessoas que você desmarcou na lista ficam de fora.

Um arquivo de mensagem só é criado para uma pessoa cujo endereço contenha um único endereço de email. Se um endereço tiver dois (com vírgula ou ponto e vírgula entre eles), tiver um nome junto, ou não tiver domínio, essa pessoa não recebe mensagem, e o JeffWrite informa quantas foram. Um assunto longo é salvo em partes, que os programas de email juntam novamente.

A mesclagem mostra uma janela de andamento, e **Cancelar** a interrompe. Se uma regra fizer uma pergunta (Perguntar ou Preenchimento), você responde a cada pergunta diferente uma única vez, até 20 perguntas. Depois disso, as demais usam as respostas padrão. Se você cancelar uma pergunta, toda a mesclagem para. A mesclagem para e informa isso se for gerar mais de 10.000 páginas, para que um documento muito longo com uma lista muito grande não leve horas. Nesse caso, mescle a lista em partes.

## Envelopes

Clique em **Correspondências > Envelopes**. Digite o endereço de entrega e o seu endereço de remetente, e escolha o tamanho do envelope na lista. Os tamanhos são indicados pela largura e pela altura. Clique em **Adicionar ao Documento** para colocar o envelope em uma página própria, no início do documento, ou em **Imprimir** para imprimi-lo. A página tem a largura do lado maior do envelope, que é como a maioria das impressoras recebe um envelope.

Para criar envelopes em uma mesclagem, clique em **Iniciar Mala Direta > Envelopes**. Cada envelope terá um bloco de endereço.

## Etiquetas

Clique em **Correspondências > Etiquetas**. Digite o endereço e escolha a folha na lista. Uma folha é identificada pela quantidade de etiquetas que tem e pelo tamanho de cada uma. Se a sua folha não estiver na lista, escolha **Sua própria folha** e digite as medidas: quantas etiquetas cabem na horizontal e na vertical, o tamanho de uma etiqueta, o espaço acima e ao lado da primeira etiqueta e a distância entre as etiquetas.

- **Página inteira da mesma etiqueta** coloca o mesmo endereço em todas as etiquetas da folha.
- **Etiqueta única** coloca o endereço em apenas uma etiqueta, para que você aproveite uma folha que já foi parcialmente usada. Informe a linha e a coluna.

Clique em **Novo Documento** para criar um documento que você possa ver e alterar, ou em **Imprimir** para imprimi-lo.

Para criar etiquetas em uma mala direta, clique em **Iniciar Mala Direta > Etiquetas**. A primeira etiqueta terá um bloco de endereço, e cada uma das outras começará com uma regra de **Próximo Registro**. Se você alterar a primeira etiqueta, clique em **Atualizar Etiquetas** para copiar a alteração para todas as outras.

## O que o arquivo guarda

O tipo de mesclagem, o nome e o local da lista, o assunto e a coluna de email são salvos no seu arquivo .docx. As pessoas que você desmarcou e as suas escolhas em Correlacionar Campos não são salvas. Essas informações são pedidas novamente depois que você abre o arquivo. Uma lista é lida de novo a partir do seu arquivo sempre que necessário. Se o arquivo não estiver mais no lugar original, o JeffWrite procura ao lado do seu documento e, se não encontrar, pede que você o escolha.

Uma lista pode ter até 1.000 colunas e 4.000.000 de células no total, e uma mesclagem usa as primeiras 100.000 pessoas. Se um arquivo tiver mais do que isso, o JeffWrite usa o que cabe e informa você. Linhas sem conteúdo são omitidas da lista.

## Veja também

- [Usar campos](fields)
- [Imprimir](print)
- [Criar e alterar tabelas](tables)
